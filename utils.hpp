#ifndef GLE_UTILS_HPP
#define GLE_UTILS_HPP

#include<map>

#define BYTE(x, n) (x) & (0xFF << (56 - 8 * (n)))
namespace GLE
{
    struct memcell
    {
        char value;
        bool is_owned;
        std::size_t owning_proc_id;
    };
    using sparse_mem_vector = std::map<std::size_t, memcell>;

    template<typename Opaque>
    int set(sparse_mem_vec& mem, const std::size_t& pid, const std::size_t& beg, const std::size_t& end, const Opaque& o)
    {
        for(std::size_t i = beg; i < end; i++)
        {
            // IF requested cell is initialised AND it is owned by a process AND
            //    owning process' ID isn't of that which requested it
            // THEN
            //    permission is denied
            // END
            if(mem.find(i) != mem.end() && mem[i].is_owned && mem[i].owning_proc_id != pid) return 1;
            mem[i] = o[i - beg];
        }
        return 0;
    }
    auto operator [](const sparse_mem_vec& mem, const std::size_t& idx)
    {
        return [idx, this](const std::size_t& pid)
        {
            if(mem.find(idx) != mem.end() && mem.at(idx).is_owned && mem.at(idx).owning_proc_id != pid)
            return mem.at(idx).value;
        };
    }
    char get(sparse_mem_vec& mem, const std::size_t& idx, const std::size_t& pid, int& ec)
    {
        if(mem.find(idx) == mem.end())
        {
            ec = 2;
            return 0;
        }
        else if(mem.find(idx) != mem.end() && mem[idx].is_owned && mem[idx].owning_proc_id != pid)
        {
            ec = 1;
            return 0;
        }
        else return mem[idx].value;
    }
    const char& get(const sparse_mem_vec& mem, const std::size_t& idx, const std::size_t& pid, int& ec)
    {
        if(mem.find(idx) == mem.end())
        {
            ec = 2;
            return 0;
        }
        else if(mem.find(idx) != mem.end() && mem.at(idx).is_owned && mem.at(idx).owning_proc_id != pid)
        {
            ec = 1;
            return 0;
        }
        else return mem.at(idx).value;
    }
}


#endif