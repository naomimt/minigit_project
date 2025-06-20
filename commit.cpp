#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>
#include <set>
#include <map>
#include <chrono>
#include <iomanip>
#include <openssl/sha.h>
#include "helpers.hpp"

namespace fs = std::filesystem;
using namespace std;

void createCommit(const string &message)
{
    string head = ".minigit/HEAD";
    string cont = readFile(head);

    cont = get_current_commit();
    string treeContent = generate_tree(cont);
    string treeHash = generateHash(treeContent);

    ofstream treeFile(".minigit/objects/" + treeHash);
    treeFile << treeContent;

    string parent = get_current_commit();
    string timestamp = get_timestamp();

    stringstream commitContent;
    commitContent << "tree " << treeHash << "\n";
    if (!parent.empty())
    {
        commitContent << "parent " << parent << "\n";
    }
    commitContent << "author " << get_author_data() << "\n";
    commitContent << "date " << timestamp << "\n";
    commitContent << "message " << message << "\n";

    string commitStr = commitContent.str();
    string commitHash = generateHash(commitStr);

    ofstream commitFile(".minigit/objects/" + commitHash);
    commitFile << commitStr;

    // 3. Update current branch
    update_current_branch(commitHash);

    // 4. Clear index
    ofstream clearIndex(".minigit/index");
    clearIndex.close();

    cout << "Committed as " << commitHash << "\n";
}
