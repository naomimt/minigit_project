#include <iostream>
#include <filesystem>
#include <fstream>
namespace fs = std::filesystem;
using namespace std;
void initMiniGit()
{
    fs::path git_dir = ".minigit";

    if (fs::exists(".minigit"))
    {
        cout << "MiniGit repository already initialized.\n";
        return;
    }

    fs::create_directory(".minigit");
    fs::create_directories(".minigit/objects");
    fs::create_directories(".minigit/refs/heads");

    ofstream headFile(".minigit/HEAD");
    headFile << "ref: refs/heads/master\n";
    headFile.close();

    ofstream masterFile(".minigit/refs/heads/master");
    masterFile << "";
    masterFile.close();

    ofstream indexFile(".minigit/index");
    indexFile << "";
    indexFile.close();

    cout << "Initialized empty MiniGit repository in .minigit/\n";
    string name, email;
    cout << "Enter author name: ";
    cin >> name;
    cout << "Enter email: ";
    cin >> email;
    ofstream author = ofstream(git_dir / "config");
    author << "[author]\n"
           << "name = " << name << "\n"
           << "email = " << email;
}
