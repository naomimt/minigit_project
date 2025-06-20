#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <ctime>

using namespace std;
// Reads the full contents of a file into a string
string readFileContents(const string &filePath)
{
    ifstream fileStream(filePath);
    if (!fileStream)
        return "";
    stringstream contentBuffer;
    contentBuffer << fileStream.rdbuf();
    return contentBuffer.str();
}

// Displays the commit history log
void printCommitLog()
{
    // Step 1: Read HEAD reference
    ifstream headFile(".minigit/HEAD");
    if (!headFile)
    {
        cerr << "fatal: HEAD not found.\n";
        return;
    }

    string headRefLine;
    getline(headFile, headRefLine);
    if (headRefLine.find("ref: ") != 0)
    {
        cerr << "fatal: invalid HEAD format.\n";
        return;
    }

    string branchRefPath = ".minigit/" + headRefLine.substr(5);
    string latestCommitHash = readFileContents(branchRefPath);
    if (latestCommitHash.empty())
    {
        cerr << "fatal: no commits yet.\n";
        return;
    }

    // Step 2: Walk through commit history
    while (!latestCommitHash.empty())
    {
        string commitObjectPath = ".minigit/objects/" + latestCommitHash;
        string commitContent = readFileContents(commitObjectPath);
        if (commitContent.empty())
        {
            cerr << "fatal: commit object not found: " << latestCommitHash << "\n";
            break;
        }

        string commitAuthor, commitDate, commitMessage, parentCommitHash;
        istringstream commitStream(commitContent);
        string commitLine;
        while (getline(commitStream, commitLine))
        {
            if (commitLine.find("author ") == 0)
            {
                commitAuthor = commitLine.substr(7);
            }
            else if (commitLine.find("date ") == 0)
            {
                commitDate = commitLine.substr(5);
            }
            else if (commitLine.find("message ") == 0)
            {
                commitMessage = commitLine.substr(8);
            }
            else if (commitLine.find("parent ") == 0)
            {
                parentCommitHash = commitLine.substr(7);
            }
        }

        cout << "commit " << latestCommitHash << "\n";
        cout << "Author: " << commitAuthor << "\n";
        cout << "Date:   " << commitDate << "\n\n";
        cout << "Message:   " << commitMessage << "\n\n";

        latestCommitHash = parentCommitHash;
    }
}
