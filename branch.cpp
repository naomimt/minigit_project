/**
 * @brief Creates a new branch in the mini-git repository.
 *
 * This function creates a new branch with the specified name by copying the latest commit hash
 * from the currently checked-out branch. It performs the following steps:
 *   - Checks if a branch with the given name already exists and aborts if it does.
 *   - Reads the HEAD file to determine the current branch reference.
 *   - Reads the latest commit hash from the current branch reference file.
 *   - Creates a new branch reference file and writes the commit hash to it.
 *
 * @param branch_name The name of the new branch to create.
 */
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

void create_branch(const string &branch_name)
{
    string new_branch_path = ".minigit/refs/heads/" + branch_name;

    // Check if the branch already exists
    ifstream check_existing_branch(new_branch_path);
    if (check_existing_branch)
    {
        cerr << "fatal: branch '" << branch_name << "' already exists.\n";
        return;
    }

    // Read HEAD to get current branch reference
    ifstream head_file(".minigit/HEAD");
    if (!head_file)
    {
        cerr << "fatal: HEAD not found.\n";
        return;
    }

    string head_ref_line;
    getline(head_file, head_ref_line);
    if (head_ref_line.find("ref: ") != 0)
    {
        cerr << "fatal: invalid HEAD format.\n";
        return;
    }

    string current_branch_ref_path = ".minigit/" + head_ref_line.substr(5); // remove "ref: "

    // Read commit hash from current branch reference
    ifstream current_branch_file(current_branch_ref_path);
    if (!current_branch_file)
    {
        cerr << "fatal: current branch file not found.\n";
        return;
    }

    string latest_commit_hash;
    getline(current_branch_file, latest_commit_hash);

    // Create new branch file and write the commit hash
    ofstream new_branch_file(new_branch_path);
    if (!new_branch_file)
    {
        cerr << "fatal: could not create branch '" << branch_name << "'.\n";
        return;
    }

    new_branch_file << latest_commit_hash;
    cout << "Branch '" << branch_name << "' created at " << latest_commit_hash << ".\n";
}
