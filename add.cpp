#include <iostream>
#include <filesystem>
#include <fstream>
#include <openssl/sha.h>
#include "helpers.hpp"

namespace fs = std::filesystem;
using namespace std;

/**
 * @brief Stages a file for the next commit in the MiniGit repository.
 *
 * This function checks if the current directory is a MiniGit repository by verifying
 * the existence of the ".minigit" directory. It then checks if the specified file exists.
 * If the file has been modified since the last commit (as determined by check_mod),
 * it creates a blob object for the file and saves its hash. The file path and blob hash
 * are then appended to the MiniGit index file for staging. Appropriate error messages
 * are displayed if any step fails.
 *
 * @param filePath The path to the file to be staged.
 */
void stageFile(const string &filePath)
{
    // Check if the .minigit directory exists to ensure we are inside a MiniGit repository
    if (!fs::exists(".minigit"))
    {
        cerr << "Error: No MiniGit repository found. Use 'init' to create one.\n";
        return;
    }

    if (!fs::exists(filePath))
    {
        cerr << "Error: File '" << filePath << "' not found.\n";
        return;
    }

    bool wasModified = check_mod(filePath);
    if (!wasModified)
    {
        cout << "File '" << filePath << "' is unchanged. No need to stage.\n";
        return;
    }
    string blobHash = saveBlobObject(filePath);
    if (blobHash.empty())
    {
        cerr << "Failed to stage file: " << filePath << "\n";
        return;
    }

    ofstream indexFile(".minigit/index", ios::app);
    if (!indexFile)
    {
        cerr << "Error: Could not update index.\n";
        return;
    }

    indexFile << filePath << " " << blobHash << "\n";
    cout << "Staged: " << filePath << " [" << blobHash << "]\n";
}
