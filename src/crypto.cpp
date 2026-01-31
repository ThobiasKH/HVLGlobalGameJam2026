static const std::string KEY = "FORMLESS_WILL_REMEMBER_YOU";

std::string DecryptTextFile(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    std::vector<char> data(
        (std::istreambuf_iterator<char>(f)),
        std::istreambuf_iterator<char>()
    );

    for (size_t i = 0; i < data.size(); ++i) {
        data[i] ^= KEY[i % KEY.size()];
    }

    return std::string(data.begin(), data.end());
}
