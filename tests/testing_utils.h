#pragma once

#include <fstream>

struct testing_utils
{
    static constexpr std::string_view secret_file = "secret";

    static void
    make_a_secret()
    {
        std::ofstream fs(secret_file.data());

        fs << "0123456789ABCDEF" << std::endl;
        fs << "FEDCBA9876543210" << std::endl;
    }
};