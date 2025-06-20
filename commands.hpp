#include <iostream>

using namespace std;
void stageFile(const string &filePath);
void create_branch(const string &branch_name);
void createCommit(const string &commitMessage);
void printCommitLog();
void initMiniGit();
void checkout(const string &ref, bool force = false);
void merge(const string &target_branch);
void diffCommits(const string &commitHash1, const string &commitHash2);