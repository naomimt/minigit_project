#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <map>
#include <vector>
#include <stdexcept>
#include <openssl/sha.h>
#include "helpers.hpp"

namespace fs = std::filesystem;
using namespace std;

string saveBlobObject(const string &filePath)
{
    ifstream inputFile(filePath);
    if (!inputFile)
    {
        cerr << "Error: Could not open file: " << filePath << "\n";
        return "";
    }

    ostringstream fileContentStream;
    fileContentStream << inputFile.rdbuf();
    string fileContent = fileContentStream.str();

    string blobHash = generateHash(fileContent);
    string objectFilePath = ".minigit/objects/" + blobHash;

    if (!fs::exists(objectFilePath))
    {
        ofstream outputFile(objectFilePath);
        if (!outputFile)
        {
            cerr << "Error: Could not write blob file.\n";
            return "";
        }
        outputFile << fileContent;
    }

    return blobHash;
}
string trim(const string &s)
{
    size_t start = s.find_first_not_of(" \n\r\t");
    size_t end = s.find_last_not_of(" \n\r\t");
    return (start == string::npos) ? "" : s.substr(start, end - start + 1);
}
string getTreeHashFromCommit(const string &commitContent)
{
    istringstream ss(commitContent);
    string line;
    while (getline(ss, line))
    {
        if (line.find("tree ", 0) == 0)
        {
            return trim(line.substr(5));
        }
    }
    return "";
}

string get_current_commit()
{
    ifstream head(".minigit/HEAD");
    string ref;
    getline(head, ref);
    if (ref.rfind("ref: ", 0) == 0)
    {
        string refPath = ref.substr(5);
        ifstream branchFile(".minigit/" + refPath);
        string commitHash;
        getline(branchFile, commitHash);
        return commitHash;
    }
    return "";
}
string readFile(const string path)
{
    ifstream file(path);
    if (!file)
        return "";
    try
    {
        return string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    }
    catch (const exception &e)
    {
        cerr << "readFile " << path << '\n';
        return "";
    }
}
bool check_mod(const string &path_to_file)
{
    string latestCommit = get_current_commit();
    if (latestCommit.empty())
        return true;
    string commitContent = readFile(".minigit/objects/" + latestCommit);
    string treeHash = getTreeHashFromCommit(commitContent);
    if (treeHash.empty())
        return true;
    string blob_path;
    istringstream treeStream(readFile(".minigit/objects/" + treeHash));
    string line;
    while (getline(treeStream, line))
    {
        if (line.find("tree ", 0) == 0)
        {
            istringstream iss(line);
            string type, hash, filename;
            iss >> type >> hash >> filename;
            if ((filename) == path_to_file)
            {
                blob_path = hash;
                break;
            }
        }
    }
    if (blob_path.empty())
    {
        return true; // File not found in the latest commit, consider it modified
    }
    else
    {
        string currentContent = trim(readFile(path_to_file));
        string blobContent = trim(readFile(".minigit/objects/" + blob_path));
        return currentContent != blobContent; // Check if content differs
    }
}
bool fileExists(const string &path)
{
    return fs::exists(path);
}
void writeFile(const string &path, const string &content)
{
    fs::path filePath(path);
    if (filePath.has_parent_path())
    {
        fs::create_directories(filePath.parent_path());
    }
    ofstream file(path);
    if (!file)
    {
        throw runtime_error("Failed to write to file: " + path);
    }
    file << content;
}

map<string, string> parseTreeObject(const string &treeContent)
{
    map<string, string> fileMap; // filename -> blobHash
    istringstream ss(treeContent);
    string line;
    while (getline(ss, line))
    {
        if (line.empty())
            continue;
        istringstream iss(line);
        string type, hash, filename;
        iss >> type >> hash >> filename;
        fileMap[filename] = hash;
    }
    return fileMap;
}
string read_index()
{
    ifstream indexFile(".minigit/index");
    stringstream ss;
    string line;
    while (getline(indexFile, line))
    {
        istringstream iss(line);
        string filename, hash;
        iss >> filename >> hash;
        ss << "blob " << hash << " " << filename << "\n";
    }
    return ss.str();
}
void update_current_branch(const string &commitHash)
{
    ifstream head(".minigit/HEAD");
    string ref;
    getline(head, ref);
    if (ref.rfind("ref: ", 0) == 0)
    {
        string refPath = ref.substr(5);
        ofstream branchFile(".minigit/" + refPath);
        branchFile << commitHash;
    }
}

string get_timestamp()
{
    auto now = chrono::system_clock::now();
    auto time = chrono::system_clock::to_time_t(now);
    ostringstream ss;
    ss << put_time(localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

string get_author_data(void)
{
    ifstream config(".minigit/config");
    string line, name, email;

    while (getline(config, line))
    {
        if (line.find("name = ") == 0)
        {
            name = line.substr(7); // skip "name = "
        }
        else if (line.find("email = ") == 0)
        {
            email = line.substr(8); // skip "email = "
        }
    }

    if (!name.empty() && !email.empty())
    {
        return name + " <" + email + ">";
    }
    else
    {
        return "Unknown <unknown@example.com>";
    }
}

string get_commit_parent(string &content)
{
    istringstream lines(content);
    string line;

    while (getline(lines, line))
    {
        if (line.find("parent ", 0) == 0)
            return line.substr(7);
    }
    return "null";
}
string generate_tree(string &current_commit_hash)
{
    map<string, string> latest_blobs; // path -> blob hash

    // Step 1: Walk commit history and collect previous tree entries
    string commit_hash = current_commit_hash;
    while (!commit_hash.empty() && commit_hash != "null")
    {
        string commit_path = ".minigit/objects/" + commit_hash;
        {
            if (!fs::exists(commit_path))
                break;

            string commit_content = readFile(commit_path);
            istringstream stream(commit_content);
            string line;

            while (getline(stream, line))
            {
                if (line.rfind("tree ", 0) == 0)
                {
                    // Format: tree <blob-hash> <path>
                    size_t hash_start = 5;
                    size_t hash_end = line.find(" ", hash_start);
                    string hash = line.substr(hash_start, hash_end);
                    string file = readFile(".minigit/objects/" + hash);
                    string path, tmp1, tmp2;
                    istringstream splitter(file);
                    splitter >> tmp1 >> tmp2 >> path;
                    cout << path << "\n";

                    if (latest_blobs.count(path) == 0)
                    {
                        latest_blobs[path] = hash;
                    }
                }
            }

            commit_hash = get_commit_parent(commit_content);
        }
    }

    // Step 2: Override with staged entries from index
    ifstream indexFile(".minigit/index");
    string line;
    while (getline(indexFile, line))
    {
        istringstream iss(line);
        string path, hash;
        iss >> path >> hash;

        if (!path.empty() && !hash.empty())
        {
            latest_blobs[path] = hash;
        }
    }

    // Step 3: Format as full tree snapshot
    stringstream result;
    for (const auto &entry : latest_blobs)
    {
        result << "tree " << entry.second << " " << entry.first << "\n";
    }

    return result.str();
}

vector<string> getModifiedFiles(const map<string, string> &committedFiles)
{
    vector<string> modifiedFiles;
    for (const auto &[filename, blobHash] : committedFiles)
    {
        if (!fileExists(filename))
        {
            modifiedFiles.push_back(filename + " (deleted)");
            continue;
        }
        string currentContent = readFile(filename);
        string blobContent = readFile(".minigit/objects/" + blobHash);
        if (currentContent != blobContent)
        {
            modifiedFiles.push_back(filename + " (modified)");
        }
    }
    return modifiedFiles;
}

map<string, string> getCurrentTrackedFiles()
{
    string headContent = readFile(".minigit/HEAD");
    string commitHash;
    if (headContent.find("ref: ") == 0)
    {
        string refPath = ".minigit/" + trim(headContent.substr(5));
        commitHash = trim(readFile(refPath));
    }
    else
    {
        commitHash = trim(headContent); // Detached HEAD
    }

    string commitContent = readFile(".minigit/objects/" + commitHash);
    string treeHash = getTreeHashFromCommit(commitContent);
    if (treeHash.empty())
        return {};

    string treeContent = readFile(".minigit/objects/" + treeHash);
    return parseTreeObject(treeContent); // filename -> blobHash
}
string generateHash(const string &content)
{
    unsigned char hashBytes[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char *>(content.c_str()), content.size(), hashBytes);

    ostringstream hashStream;
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i)
    {
        hashStream << hex << setw(2) << setfill('0') << (int)hashBytes[i];
    }
    return hashStream.str();
}