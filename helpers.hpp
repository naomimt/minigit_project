#include <iostream>
#include <map>
#include <vector>

using namespace std;
std::string saveBlobObject(const std::string &filePath);
string trim(const string &s);
string getTreeHashFromCommit(const string &commitContent);
string get_current_commit();
string readFile(const string path);
bool check_mod(const string &path_to_file);
bool fileExists(const string &path);
void writeFile(const string &path, const string &content);
string read_index();
string generate_tree(string &current_commit_hash);
vector<string> getModifiedFiles(const map<string, string> &committedFiles);
string get_commit_parent(string &content);
std::string generateHash(const std::string &content);
map<string, string> getCurrentTrackedFiles();
string get_author_data(void);
string get_timestamp();
void update_current_branch(const string &commitHash);
map<string, string> parseTreeObject(const string &treeContent);