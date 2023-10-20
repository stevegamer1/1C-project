#include <filesystem>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <fstream>


// top-level project namespace
namespace jarvis {

using uint32_vec = std::vector<uint32_t>;
using uint32_matrix = std::vector<std::vector<uint32_t>>;

double Uniform01() {
    return std::rand() / RAND_MAX;
}

bool BernoulliHalf() {
    return std::rand() % 2 == 0;
}

// return true if the directory is ready to work with
// otherwise print error to screen and return false
bool CheckDir(std::string dirname) {
    std::error_code error;
    bool result = std::filesystem::exists(dirname, error);

    if (!result) {
        std::cout << "error: directory '" << dirname << "' doesn't exist. code: " << error << "\n";
    }

    return result;
}

class CompareFilesize {
public:
    bool operator()(const std::string& first, const std::string& second) {
        return std::filesystem::file_size(first) > std::filesystem::file_size(second);
    }
};


class DirectoryComparator {
public:
    // compare without checking directory validity
    void CompareNocheck(std::string dir1, std::string dir2) {
        // initialize
        for (const auto& entry : std::filesystem::directory_iterator(dir1)) {
            dir1Filenames.push_back(entry.path());
        }
        for (const auto& entry : std::filesystem::directory_iterator(dir2)) {
            dir2Filenames.push_back(entry.path());
        }

        std::sort(dir1Filenames.begin(), dir1Filenames.end(), CompareFilesize());
        std::sort(dir2Filenames.begin(), dir2Filenames.end(), CompareFilesize());

        uint32_matrix distances = uint32_matrix(dir1Filenames.size(),
                                     uint32_vec(dir2Filenames.size(), 0));


        // compare
        // should've used some file pool here, but didn't have time
        for (size_t file1 = 0; file1 < dir1Filenames.size(); ++file1) {
            std::ifstream ifstr1(dir1Filenames[file1]);
            std::stringstream stream1;
            stream1 << ifstr1.rdbuf();
            std::string data1 = stream1.str();
            for (size_t file2 = 0; file2 < dir2Filenames.size(); ++file2) {
                std::ifstream ifstr2(dir2Filenames[file2]);
                std::stringstream stream2;
                stream2 << ifstr2.rdbuf();
                std::string data2 = stream1.str();
                distances[file1][file2] = RandomDist(data1, data2);
            }
        }


        // print result
        for (size_t file1 = 0; file1 < dir1Filenames.size(); ++file1) {
            for (size_t file2 = 0; file2 < dir2Filenames.size(); ++file2) {
                std::cout << dir1Filenames[file1] << " vs " << dir2Filenames[file2] << ": ";
                std::cout << distances[file1][file2] << " bytes\n";
            }
        }
    }

private:
    std::vector<std::string> dir1Filenames;
    std::vector<std::string> dir2Filenames;

    // returns sum of equal parts lengths
    uint32_t RandomAttemptTillEnd(const std::string& data1, const std::string& data2,
                                  uint32_t start1, uint32_t start2) {
        uint32_t result = 0;
        auto i = data1.begin() + start1;
        auto j = data2.begin() + start2;
                
        while (i != data1.end() && j != data2.end()) {
            while(i != data1.end() && j != data2.end() && *i != *j) {
                if (BernoulliHalf()) {
                    ++i;
                } else {
                    ++j;
                }
            }

            if (i != data1.end() && j != data2.end()) {
                // yay! we've found a similar part!
                uint32_t similarPartLen = 0;
                while(i != data1.end() && j != data2.end() && *i == *j) {
                    ++i;
                    ++j;
                    ++result;
                }
            }
        }
        return result;
    }

    // distance measured using randomness-based heuristic
    uint32_t RandomDist(const std::string& data1, const std::string& data2) {
        uint32_t minLen = std::min(data1.size(), data2.size());
        uint32_t equalityEnd = 0;  // where leading equal sequences end?
        while (equalityEnd < minLen && data1[equalityEnd] == data2[equalityEnd]) {
            ++equalityEnd;
        }
        // now we know that strings are equal up to byte number equalityEnd

        uint32_t result = equalityEnd;
        if (equalityEnd < minLen) {
            // how many times to do random walk
            uint32_t randomAttempts = std::sqrt(std::max(data1.size(), data2.size()));
            /* auto attemptStart1 = data1.begin() + equalityEnd;
            auto attemptStart2 = data2.begin() + equalityEnd; */

            for (uint32_t att = 0; att < randomAttempts; ++att) {
                uint32_t attemptResult = equalityEnd +
                    RandomAttemptTillEnd(data1, data2, equalityEnd, equalityEnd);
                result = std::max(result, attemptResult);
            }
        }

        return result;
    }
};


// Take command line arguments and execute accordingly
void Solve() {
    std::string dir1;
    std::string dir2;
    std::cout << "Hello, user! Please provide me with two directories and I will compare their files:\n";
    std::cin >> dir1;
    std::cin >> dir2;

    if (CheckDir(dir1) && CheckDir(dir2)) {
        DirectoryComparator comp;

        comp.CompareNocheck(dir1, dir2);
    }
}

}

int main() {
    jarvis::Solve();
}