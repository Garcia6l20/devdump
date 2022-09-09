#include <charconv>
#include <string_view>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cctype>
#include <cstdlib>

extern "C"
{
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
}

static constexpr size_t map_size = 4096;
static constexpr size_t map_mask = map_size - 1;

class mem
{
public:
    mem(off_t address_start, off_t address_end) : address_base_{address_start}
    {
        fd_ = ::open("/dev/mem", O_RDWR | O_SYNC);
        if (fd_ == -1)
        {
            throw std::runtime_error("Cannot open /dev/mem");
        }
        void *map_base = ::mmap(nullptr, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, address_start & ~map_mask);
        if (map_base == MAP_FAILED)
        {
            throw std::runtime_error("Cannot create mem mapping");
        }
        mmap_ = reinterpret_cast<uint8_t *>(map_base);
    }
    ~mem() noexcept
    {
        if (fd_ != -1)
        {
            ::close(fd_);
        }
        if (mmap_ != nullptr)
        {
            ::munmap(mmap_, map_size);
        }
    }
    off_t operator[](off_t address) const
    {
        return *reinterpret_cast<off_t *>(mmap_ + (address & map_mask));
    }

private:
    int fd_ = -1;
    off_t address_base_;
    uint8_t *mmap_;
};

int main(int argc, char **argv)
{
    std::vector<std::string_view> args{argv + 1, argv + argc};
    if (args.size() == 0)
    {
        std::cerr << "missing address list\n";
        std::exit(-1);
    }
    std::vector<off_t> addresses;
    for (auto const &arg : args)
    {
        off_t address = 0;
        auto start = arg.begin();
        int base = 10;
        if (arg[0] == '0' and std::tolower(arg[1]) == 'x')
        {
            start += 2;
            base = 16;
        }
        auto [_, ok] = std::from_chars(start, arg.end(), address, base);
        if (ok != std::errc{})
        {
            std::cerr << "cannot parse '" << arg << "': " << std::make_error_code(ok).message() << '\n';
            std::exit(-1);
        }
        addresses.push_back(address);
    }
    std::sort(addresses.begin(), addresses.end());
    mem map{addresses.at(0), addresses.at(addresses.size() - 1)};
    for (const auto address : addresses)
    {
        const auto value = map[address];
        std::cout << "0x" << std::hex << std::setw(8) << std::setfill('0') << address
                  << " => 0x" << std::hex << std::setw(8) << std::setfill('0') << value << '\n';
    }
    return 0;
}
