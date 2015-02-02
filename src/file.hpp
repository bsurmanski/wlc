#include <fstream>
#include <iostream>

std::string findFile(std::string filenm);

class File {
    std::string filename;
    std::ifstream stream;

    public:
    File(std::string filenm) : stream(findFile(filenm)), filename(filenm) {
    }

    std::string getName() {
        return filename;
    }

    std::ifstream &getStream() {
        return stream;
    }

    bool exists() {
        return !stream.fail();
    }
};
