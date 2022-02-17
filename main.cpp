#include <cstdlib>
#include <iostream>

#include <redis-cpp/stream.h>
#include <redis-cpp/execute.h>
#include <chrono>

// https://github.com/tdv/redis-cpp

int main()
{
    try
    {
        auto stream = rediscpp::make_stream("localhost", "6379");

        auto const key = "my_key";

        auto response = rediscpp::execute(*stream, "set",
                                          key, "Some value for 'my_key'", "ex", "60");

        std::cout << "Set key '" << key << "': " << response.as<std::string>() << std::endl;

        response = rediscpp::execute(*stream, "get", key);
        std::cout << "Get key '" << key << "': " << response.as<std::string>() << std::endl;

        auto start = std::chrono::high_resolution_clock::now();

        int const N = 1000000;
        auto const key_pref = "my_key_";

        // Executing command 'SET' N times without getting any response
        for (int i = 0; i < N; ++i)
        {
            auto const item = std::to_string(i);
            rediscpp::execute_no_flush(*stream,
                                       "set", key_pref + item, item, "ex", "60");
        }

        // Flush all
        std::flush(*stream);

        // Getting response for each sent 'SET' request
        for (int i = 0; i < N; ++i)
        {
            rediscpp::value value{*stream};
            //std::cout << "Set " << key_pref << i << ": " << value.as<std::string_view>() << std::endl;
        }

        // Executing command 'GET' N times without getting any response
        for (int i = 0; i < N; ++i)
        {
            rediscpp::execute_no_flush(*stream, "get",
                                       key_pref + std::to_string(i));
        }

        // Flush all
        std::flush(*stream);

        // Getting response for each sent 'GET' request
        for (int i = 0; i < N; ++i)
        {
            rediscpp::value value{*stream};
            //std::cout << "Get " << key_pref << i << ": " << value.as<std::string_view>() << std::endl;
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Duration: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << std::endl;
    }
    catch (std::exception const &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}