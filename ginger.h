#ifndef GINGER_H_INCLUDED
#define GINGER_H_INCLUDED

#include <iostream>
#include <exception>
#include <sstream>
#include <string>
#include <utility>
#include <map>
#include <vector>
#include <cassert>
#include <iterator>

namespace ginger {

class object;
class iterator {
    struct holder;
    template<class T> struct holder_impl;

    std::shared_ptr<holder> holder_;
public:
    template<class T> iterator(T it);

    void operator++();
    object operator*();
    bool operator!=(iterator it) const;
};

class object {
    struct holder {
        virtual ~holder() { }
        virtual bool cond() const = 0;
        virtual iterator begin() = 0;
        virtual iterator end() = 0;
        virtual std::string str() const = 0;
        virtual object get(std::string name) = 0;
    };

    template<class T>
    struct holder_impl : holder {
        T obj;
        holder_impl(T obj) : obj(std::move(obj)) { }

        template<class U, int = sizeof(std::declval<U>() ? true : false)>
        bool cond_(int) const {
            return static_cast<bool>(obj);
        }
        template<class>
        bool cond_(...) const {
            throw "This value does not evaluate.";
        }
        virtual bool cond() const override {
            return cond_<T>(0);
        }

        template<class U, std::size_t = sizeof(std::declval<U>().begin(), std::declval<U>().end())>
        iterator begin_(short) {
            return obj.begin();
        }
        template<class>
        iterator begin_(...) {
            throw "This value does not have begin().";
        }
        virtual iterator begin() override {
            return begin_<T>(0);
        };

        template<class U, std::size_t = sizeof(std::declval<U>().begin(), std::declval<U>().end())>
        iterator end_(short) {
            return obj.end();
        }
        template<class>
        iterator end_(...) {
            throw "This value does not have end().";
        }
        virtual iterator end() override {
            return end_<T>(0);
        }

        template<class U, int = sizeof(std::declval<std::stringstream&>() << std::declval<U>())>
        std::string str_(int) const {
            std::stringstream ss;
            ss << obj;
            return ss.str();
        }
        template<class U>
        std::string str_(...) const {
            throw "This value does not have operator<<().";
        }
        virtual std::string str() const override {
            return str_<T>(0);
        }

        template<class U, int = sizeof(std::declval<U>()[std::declval<std::string>()])>
        object get_(int, std::string name) {
            return obj[std::move(name)];
        }
        template<class U>
        object get_(long, std::string, ...) {
            throw "This value does not have operator[]().";
        }
        virtual object get(std::string name) override {
            return get_<T>(0, std::move(name));
        }
    };

    template<class Cond, class Begin, class End, class Str, class Get>
    struct holder_impl2 : holder {
        Cond fcond_;
        Begin fbegin_;
        End fend_;
        Str fstr_;
        Get fget_;

        holder_impl2(Cond cond, Begin begin, End end, Str str, Get get)
            : fcond_(std::move(cond))
            , fbegin_(std::move(begin))
            , fend_(std::move(end))
            , fstr_(std::move(str))
            , fget_(std::move(get)) { }

        template<class U, int = sizeof(std::declval<U>()())>
        bool cond_(int) const {
            return fcond_();
        }
        template<class>
        bool cond_(...) const {
            throw __LINE__;
        }
        virtual bool cond() const override {
            return cond_<Cond>();
        }

        template<class U, int = sizeof(std::declval<U>()())>
        bool begin_(int) const {
            return fbegin_();
        }
        template<class>
        bool begin_(...) const {
            throw __LINE__;
        }
        virtual iterator begin() override {
            return begin_<Begin>();
        }

        template<class U, int = sizeof(std::declval<U>()())>
        bool xend_(int) const {
            return fend_();
        }
        template<class>
        bool xend_(...) const {
            throw __LINE__;
        }
        virtual iterator end() override {
            return xend_<End>();
        }

        template<class U, int = sizeof(std::declval<U>()())>
        bool str_(int) const {
            return fstr_();
        }
        template<class>
        bool str_(...) const {
            throw __LINE__;
        }
        virtual std::string str() const override {
            return str_<Str>();
        }

        template<class U, int = sizeof(std::declval<U>()())>
        bool get_(int, std::string name) const {
            return fget_(std::move(name));
        }
        template<class>
        bool get_(long, std::string, ...) const {
            throw __LINE__;
        }
        virtual object get(std::string name) override {
            return get_<Get>();
        }
    };

    std::shared_ptr<holder> holder_;

public:
    object() = default;
    object(const object&) = default;
    object(object&&) = default;
    object& operator=(const object&) = default;
    object& operator=(object&&) = default;

    template<class T>
    object(T v) : holder_(new holder_impl<T>(std::move(v))) { }

    template<class T>
    void operator=(T v) { holder_.reset(new holder_impl<T>(std::move(v))); }

    explicit operator bool() const { return holder_->cond(); }
    iterator begin() { return holder_->begin(); }
    iterator end() { return holder_->end(); }
    std::string str() const { return holder_->str(); }
    object operator[](std::string name) { return holder_->get(std::move(name)); }

    template<class Cond, class Begin, class End, class Str, class Get>
    static object create(Cond cond, Begin begin, End end, Str str, Get get) {
        object obj;
        obj.holder_.reset(new holder_impl2<Cond, Begin, End, Str, Get>(std::move(cond), std::move(begin), std::move(end), std::move(str), std::move(get)));
        return obj;
    }
};

struct iterator::holder {
    virtual ~holder() { }
    virtual void operator++() = 0;
    virtual object operator*() = 0;
    virtual bool operator!=(iterator it) const = 0;
};
template<class T>
struct iterator::holder_impl : iterator::holder {
    T it;
    holder_impl(T it) : it(std::move(it)) { }
    virtual void operator++() override {
        ++it;
    }
    virtual object operator*() override {
        return *it;
    }
    virtual bool operator!=(iterator it) const override {
        return this->it != std::static_pointer_cast<holder_impl<T>>(it.holder_)->it;
    }
};
template<class T>
iterator::iterator(T it)
    : holder_(new iterator::holder_impl<T>(std::move(it))) { }

void iterator::operator++() {
    holder_->operator++();
}
object iterator::operator*() {
    return holder_->operator*();
}
bool iterator::operator!=(iterator it) const {
    return holder_->operator!=(it);
}

typedef std::map<std::string, object> temple;

class parse_error : public std::exception {
    std::string message_;
    int line_number_;
    std::string line1_;
    std::string line2_;
    std::string line_;
    std::string long_error_;

public:
    parse_error(std::string message, int line_number, std::string line1, std::string line2)
        : message_(std::move(message))
        , line_number_(line_number)
        , line1_(line1)
        , line2_(line2) {
        {
            std::stringstream ss;
            ss << "line " << line_number_ << ": " << line1_ << line2_ << std::endl;
            line_ = ss.str();
        }
        {
            std::stringstream ss;
            ss << "ERROR: " << message_ << "\n";
            ss << "LINE: " << line_number_ << "\n";
            ss << line1_ << line2_ << std::endl;
            ss << std::string(line1_.length(), ' ') << '^' << "  <-- current cursor is here\n";
            long_error_ = ss.str();
        }
    }
    virtual const char* what() const noexcept { return line_.c_str(); }
    int line_number() const noexcept { return line_number_; }
    const std::string& line1() const noexcept { return line1_; }
    const std::string& line2() const noexcept { return line2_; }
    const std::string& long_error() const noexcept { return long_error_; }
};

namespace internal {

typedef std::map<std::string, std::vector<object>> tmpl_context;

template<class Iterator>
class parser {
public:
    Iterator current_;
    Iterator next_;
    Iterator last_;
    std::string line_;
    int line_number_;

public:
    typedef std::pair<Iterator, Iterator> range_t;

    parser(Iterator first, Iterator last) : next_(first), last_(last), line_number_(1) {
        assert(first != last);
        current_ = next_++;
    }

    void read() {
        if (current_ == last_)
            throw std::string("End of string suddenly at read()");
        if (*current_ == '\n') {
            line_.clear();
            ++line_number_;
        } else {
            line_.push_back(*current_);
        }
        current_ = next_;
        if (next_ != last_)
            ++next_;
    }
    parse_error read_error(std::string message) {
        std::string line;
        while (current_ != last_ && *current_ != '\n')
            line.push_back(*current_++);
        return parse_error(std::move(message), line_number_, line_, line);
    }

    explicit operator bool() const {
        return current_ != last_;
    }
    char peek() const {
        if (current_ == last_)
            throw std::string("Do not access end of string");
        return *current_;
    }
    bool has_next() const {
        return next_ != last_;
    }
    char next() const {
        if (next_ == last_)
            throw std::string("Next value is already end of string");
        return *next_;
    }

    typedef std::tuple<Iterator, std::string, int> context_t;
    context_t save() const {
        return std::make_tuple(current_, line_, line_number_);
    }
    void load(context_t context) {
        current_ = next_ = std::get<0>(context);
        if (next_ != last_)
            ++next_;
        line_ = std::get<1>(context);
        line_number_ = std::get<2>(context);
    }

    template<class F>
    range_t read_while(F f) {
        if (current_ == last_)
            throw std::string("End of string suddenly at read_while()");
        auto first = current_;
        while (f(peek()))
            read();
        return std::make_pair(first, current_);
    }
    template<class F>
    range_t read_while_or_eof(F f) {
        auto first = current_;
        while (current_ != last_ && f(*current_))
            read();
        return std::make_pair(first, current_);
    }

    void skip_whitespace() {
        read_while([](char c) { return c <= 32; });
    }
    void skip_whitespace_or_eof() {
        read_while_or_eof([](char c) { return c <= 32; });
    }
    range_t read_ident() {
        skip_whitespace();
        return read_while([](char c) { return c > 32 && c != '{' && c != '}'; });
    }
    std::string read_ident_str() {
        auto r = read_ident();
        return std::string(r.first, r.second);
    }
    range_t read_variable() {
        auto r = read_while([](char c) { return c > 32 && c != '.' && c != '{' && c != '}'; });
        if (r.first == r.second)
            throw std::string("Did not find variable at read_variable().");
        return r;
    }
    std::string read_variable_str() {
        auto r = read_variable();
        return std::string(r.first, r.second);
    }

    void eat(char c) {
        if (peek() != c)
            throw std::string("Unexpected character ") + peek() + ". Expected character is " + c;
        read();
    }
    void eat(const char* p) {
        while (*p)
            eat(*p++);
    }
    void eat_with_whitespace(const char* p) {
        skip_whitespace();
        eat(p);
    }
    static bool equal(std::pair<Iterator, Iterator> p, const char* str) {
        while (p.first != p.second && *str)
            if (*p.first++ != *str++)
                return false;
        return p.first == p.second && not *str;
    }
};

template<class F, std::size_t N>
void output_string(F& out, const char (&s)[N]) {
    out.put(s, s + N - 1);
}

template<class Iterator, class Dict>
object get_variable(parser<Iterator>& p, const Dict& dic, tmpl_context& ctx, bool skip) {
    p.skip_whitespace();
    if (skip) {
        p.read_variable();
        while (p.peek() == '.') {
            p.read();
            p.read_variable();
        }
        return object();
    } else {
        std::string var = p.read_variable_str();
        auto it = ctx.find(var);
        object obj;
        if (it != ctx.end() && not it->second.empty()) {
            obj = it->second.back();
        } else {
            auto it2 = dic.find(var);
            if (it2 != dic.end()) {
                obj = it2->second;
            } else {
                throw std::string("Variable \"") + var + "\" is not found";
            }
        }
        while (p.peek() == '.') {
            p.read();
            std::string var = p.read_variable_str();
            obj = obj[var];
        }
        return obj;
    }
}

template<class Iterator, class Dict, class F>
void block(parser<Iterator>& p, const Dict& dic, tmpl_context& ctx, bool skip, F& out) {
    while (p) {
        auto r = p.read_while_or_eof([](char c) { return c != '}' && c != '$'; });
        if (not skip)
            out.put(r.first, r.second);
        if (!p)
            break;

        char c = p.peek();
        if (c == '}') {
            if (p.has_next() && p.next() == '}') {
                // end of block
                break;
            } else {
                p.read();
                if (not skip)
                    output_string(out, "}");
            }
        } else if (c == '$') {
            p.read();
            c = p.peek();
            if (c == '$') {
                // $$
                p.read();
                if (not skip)
                    output_string(out, "$");
            } else if (c == '#') {
                // $# comments
                p.read_while_or_eof([](char peek) {
                    return peek != '\n';
                });
            } else if (c == '{') {
                p.read();
                c = p.peek();
                if (c == '{') {
                    // ${{
                    p.read();
                    if (not skip)
                        output_string(out, "{{");
                } else {
                    // ${variable}
                    object obj = get_variable(p, dic, ctx, skip);
                    p.eat_with_whitespace("}");

                    if (not skip) {
                        std::string str = obj.str();
                        out.put(str.begin(), str.end());
                    }
                }
            } else if (c == '}') {
                p.read();
                c = p.peek();
                if (c == '}') {
                    // $}}
                    p.read();
                    if (not skip)
                        output_string(out, "}}");
                } else {
                    throw std::string("Unexpected character '") + c + "'. It must be '}' after \"$}\"";
                }
            } else {
                auto command = p.read_ident();
                if (p.equal(command, "for")) {
                    // $for x in xs {{ <block> }}
                    auto var1 = p.read_ident_str();
                    auto in = p.read_ident();
                    if (not p.equal(in, "in"))
                        throw "Unexpected string \"" + std::string(in.first, in.second) + "\". It must be \"in\"";
                    object obj = get_variable(p, dic, ctx, skip);
                    p.eat_with_whitespace("{{");

                    if (skip) {
                        block(p, dic, ctx, true, out);
                    } else {
                        auto context = p.save();
                        auto& vec = ctx[var1];
                        for (object v: obj) {
                            vec.push_back(v);
                            block(p, dic, ctx, skip, out);
                            vec.pop_back();
                            p.load(context);
                        }
                        block(p, dic, ctx, true, out);
                    }
                    p.eat("}}");
                } else if (p.equal(command, "if")) {
                    // $if x {{ <block> }}
                    // $elseif y {{ <block> }}
                    // $elseif z {{ <block> }}
                    // $else {{ <block> }}
                    object obj = get_variable(p, dic, ctx, skip);
                    p.eat_with_whitespace("{{");
                    bool run; // if `skip` is true, `run` is an unspecified value.
                    if (skip) {
                        block(p, dic, ctx, true, out);
                    } else {
                        run = static_cast<bool>(obj);
                        block(p, dic, ctx, not run, out);
                    }
                    p.eat("}}");
                    while (true) {
                        auto context = p.save();
                        p.skip_whitespace_or_eof();
                        if (!p) {
                            p.load(context);
                            break;
                        }
                        c = p.peek();
                        if (c == '$') {
                            p.read();
                            auto command = p.read_ident();
                            if (p.equal(command, "elseif")) {
                                object obj = get_variable(p, dic, ctx, skip || run);
                                p.eat_with_whitespace("{{");
                                if (skip || run) {
                                    block(p, dic, ctx, true, out);
                                } else {
                                    bool run_ = static_cast<bool>(obj);
                                    block(p, dic, ctx, not run_, out);
                                    if (run_)
                                        run = true;
                                }
                                p.eat("}}");
                            } else if (p.equal(command, "else")) {
                                p.eat_with_whitespace("{{");
                                block(p, dic, ctx, skip || run, out);
                                p.eat("}}");
                                break;
                            } else {
                                p.load(context);
                                break;
                            }
                        } else {
                            p.load(context);
                            break;
                        }
                    }
                } else {
                    throw "Unexpected command " + std::string(command.first, command.second) + ". It must be \"for\" or \"if\"";
                }
            }
        } else {
            assert(false && "must not go through.");
            throw "Must not go through.";
        }
    }
}

template<class IOS>
struct ios_type {
    IOS ios;
    ios_type(IOS ios) : ios(std::forward<IOS>(ios)) { }

    template<class Iterator>
    void put(Iterator first, Iterator last) {
        std::copy(first, last, std::ostreambuf_iterator<char>(ios));
    }
    void flush() {
        ios << std::flush;
    }

    ios_type(ios_type&&) = default;
    ios_type& operator=(ios_type&&) = default;

    ios_type(const ios_type&) = delete;
    ios_type& operator=(const ios_type&) = delete;
};

struct cstring : std::iterator<std::forward_iterator_tag, char> {
    cstring() : p(nullptr) { }
    cstring(const char* p) : p(p) { }
    cstring(const cstring& c) : p(c.p) { }
    cstring begin() { return cstring(p); }
    cstring end() { return cstring(); }
    cstring& operator++() {
        assert(*p != '\0');
        ++p;
        return *this;
    }
    cstring operator++(int) {
        assert(*p != '\0');
        cstring t = *this;
        ++p;
        return t;
    }
    const char& operator*() {
        return *p;
    }
    const char& operator*() const {
        return *p;
    }
    friend bool operator==(const cstring& a, const cstring& b) {
        return
            not a.p && not b.p ? true :
            not a.p &&     b.p ? *b.p == '\0' :
                a.p && not b.p ? *a.p == '\0' :
                                 a.p == b.p;
    }
    friend bool operator!=(const cstring& a, const cstring& b) {
        return !(a == b);
    }
private:
    const char* p;
};

}

template<class IOS>
internal::ios_type<IOS> from_ios(IOS&& ios) {
    return internal::ios_type<IOS>(std::forward<IOS>(ios));
}

template<class Iterator>
internal::parser<Iterator> make_parser(Iterator first, Iterator last) {
    return internal::parser<Iterator>(first, last);
}
template<class Input, class Dict, class F>
static void parse(Input&& input, Dict&& t, F&& out) {
    auto first = std::begin(input);
    auto last = std::end(input);
    if (first == last) return;

    auto p = make_parser(first, last);
    internal::tmpl_context ctx;
    try {
        internal::block(p, t, ctx, false, out);
    } catch (std::string message) {
        throw p.read_error(std::move(message));
    } catch (...) {
        throw p.read_error("unexpected error");
    }
    out.flush();
}
template<class Input, class Dict>
static void parse(Input&& input, Dict&& t) {
    parse(std::forward<Input>(input), std::forward<Dict>(t), from_ios(std::cout));
}
template<class Dict, class F>
static void parse(const char* input, Dict&& t, F&& out) {
    parse(internal::cstring(input), std::forward<Dict>(t), std::forward<F>(out));
}
template<class Dict>
static void parse(const char* input, Dict&& t) {
    parse(internal::cstring(input), std::forward<Dict>(t), from_ios(std::cout));
}

}

#endif // GINGER_H_INCLUDED
