#include <iostream>

class membuf : public std::streambuf {
public:
  membuf(const char* p, size_t l) {
    setg((char*)p, (char*)p, (char*)p + l);
  }

  std::streampos seekpos(std::streampos new_position, 
    std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out)
  {
    _M_in_cur = _M_in_beg + new_position;
    return new_position;
  }

  std::streampos seekoff(std::streamoff off, std::ios_base::seekdir way,
	  std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out)
  {
    char* loc = nullptr;
    switch (way)
    {
    case std::ios_base::beg:
      loc = _M_in_beg;
      break;
    case std::ios_base::cur:
      loc = _M_in_cur;
      break;
    case std::ios_base::end:
      loc = _M_in_end;
      break;
    };
    _M_in_cur = loc + off;
    return _M_in_cur - _M_in_beg;
  }
  
};

class memstream : public std::istream {
public:
  memstream(const char* p, size_t l) :
    std::istream(&_buffer),
    _buffer(p, l) {
    rdbuf(&_buffer);
  }

private:
  membuf _buffer;
};