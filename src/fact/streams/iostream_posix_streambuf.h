//
// Created by malachi on 3/19/17.
//

#ifndef UTIL_EMBEDDED_TESTS_IOSTREAM_POSIX_STREAMBUF_H
#define UTIL_EMBEDDED_TESTS_IOSTREAM_POSIX_STREAMBUF_H

// specifically for non-FEATURE_IOS_STREAMBUF_FULL mode

//namespace FactUtilEmbedded { namespace std {

//namespace experimental {

template<class TChar, class Traits = char_traits <TChar>>
class basic_streambuf : public experimental::basic_streambuf_embedded<TChar, ::_IO_FILE, Traits>
{
protected:
    typedef experimental::basic_streambuf_embedded<TChar, ::_IO_FILE, Traits> base_t;

public:
    typedef TChar char_type;

    basic_streambuf(_IO_FILE &stream) : base_t(stream)
    {}

    streamsize xsputn(const char_type *s, streamsize count)
    {
        return fwrite(s, sizeof(TChar), count, &this->stream);
    }

    streamsize xsgetn(char_type *s, streamsize count)
    {
        return fread(s, sizeof(TChar), count, &this->stream);
    }
};

//}

//} }

#endif //UTIL_EMBEDDED_TESTS_IOSTREAM_POSIX_STREAMBUF_H
