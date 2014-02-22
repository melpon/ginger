#include <iostream>
#include <ios>
#include <string>
#include <utility>
#include <map>
#include <vector>
#include <cassert>

std::string getStdin() {
    std::string output;
    std::string input;
    std::cin >> std::noskipws;
    while (std::getline(std::cin, input)) {
        output += input + '\n';
    }
    return output;
}

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
        virtual iterator xbegin() = 0;
        virtual iterator xend() = 0;
        virtual std::string str() const = 0;
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
            throw __LINE__;
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
            throw __LINE__;
        }
        virtual iterator xbegin() override {
            return begin_<T>(0);
        };

        template<class U, std::size_t = sizeof(std::declval<U>().begin(), std::declval<U>().end())>
        iterator end_(short) {
            return obj.end();
        }
        template<class>
        iterator end_(...) {
            throw __LINE__;
        }
        virtual iterator xend() override {
            return end_<T>(0);
        }

        template<class U, int = sizeof(std::to_string(std::declval<U>()))>
        std::string str_(int) const {
            return std::to_string(obj);
        }
        template<class U>
        std::string str_(...) const {
            throw __LINE__;
        }
        virtual std::string str() const override {
            return str_<T>(0);
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
    iterator xbegin() { return holder_->xbegin(); }
    iterator xend() { return holder_->xend(); }
    std::string str() const { return holder_->str(); }
};

iterator begin(object& obj) {
    return obj.xbegin();
}
iterator end(object& obj) {
    return obj.xend();
}

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

typedef std::map<std::string, object> tmpl;
typedef std::map<std::string, std::vector<object>> tmpl_context;

template<class Iterator>
class parser {
public:
    Iterator current_;
    Iterator next_;
    Iterator last_;

    void skip_(int n, std::forward_iterator_tag) {
        while (n != 0) {
            read();
            --n;
        }
    }
    void skip_(int n, std::random_access_iterator_tag) {
        if (last_ - current_ < n)
            throw __LINE__;
        current_ += n;
        next_ += n - 1;
        if (next_ != last_)
            ++next_;
    }

public:
    typedef std::pair<Iterator, Iterator> range_t;

    parser(Iterator first, Iterator last) : next_(first), last_(last) {
        assert(first != last);
        current_ = next_++;
    }

    void skip(int n) {
        skip_(n, typename std::iterator_traits<Iterator>::iterator_category());
    }
    void read() {
        if (current_ == last_)
            throw __LINE__;
        current_ = next_;
        if (next_ != last_)
            ++next_;
    }

    explicit operator bool() const {
        return current_ != last_;
    }
    char peek() const {
        if (current_ == last_)
            throw __LINE__;
        return *current_;
    }
    bool has_next() const {
        return next_ != last_;
    }
    char next() const {
        if (next_ == last_)
            throw __LINE__;
        return *next_;
    }

    Iterator save() const {
        return current_;
    }
    void load(Iterator context) {
        current_ = next_ = context;
        if (next_ != last_)
            ++next_;
    }

    template<class F>
    range_t read_while(F f) {
        if (current_ == last_)
            throw __LINE__;
        auto first = current_;
        while (f(*current_))
            read();
        return std::make_pair(first, current_);
    }
    template<class F>
    range_t read_while_next(F f) {
        if (current_ == last_)
            throw __LINE__;
        auto first = current_;
        while (f(*current_, has_next(), has_next() ? *next_ : 0))
            read();
        return std::make_pair(first, current_);
    }
    void eat(char c) {
        if (peek() != c)
            throw __LINE__;
        read();
    }
    template<class F>
    void eat_f(F f) {
        if (not f(peek()))
            throw __LINE__;
        read();
    }
};

template<class Iterator>
void block(parser<Iterator>& p, tmpl& dic, tmpl_context& ctx, bool not_skip=true) {
    while (p) {
        char c = p.peek();
        if (c == '}' && p.has_next() && p.next() == '}') {
            break;
        } else if (c != '$') {
            if (not_skip) std::cout << c;
            p.read();
        } else {
            p.read();
            c = p.peek();
            if (c == '$') {
                p.read();
                if (not_skip) std::cout << '$';
            } else if (c == '#') {
                // skip comments
                p.read_while([](char peek) {
                    return peek != '\n';
                });
            } else if (c == '{') {
                p.read();
                c = p.peek();
                if (c == '{') {
                    p.read();
                    if (not_skip) std::cout << "{{";
                } else {
                    auto r = p.read_while([](char peek) {
                        return peek != '}';
                    });
                    p.eat('}');

                    if (not_skip) {
                        std::string var = std::string(r.first, r.second);
                        auto it = ctx.find(var);
                        if (it != ctx.end() && not it->second.empty()) {
                            std::cout << it->second.back().str();
                        } else {
                            std::cout << dic[var].str();
                        }
                    }
                }
            } else if (c == '}') {
                p.read();
                c = p.peek();
                if (c == '}') {
                    p.read();
                    if (not_skip) std::cout << "}}";
                } else {
                    throw __LINE__;
                }
            } else {
                auto ws = [](char peek) { return peek <= 32; };
                auto token = [](char peek) { return peek > 32; };

                auto r = p.read_while(token);
                std::string command(r.first, r.second);

                if (command == "for") {
                    p.read_while(ws);
                    r = p.read_while(token);
                    std::string var1(r.first, r.second);;
                    p.read_while(ws);
                    p.eat('i');
                    p.eat('n');
                    p.eat_f(ws);
                    p.read_while(ws);
                    r = p.read_while([token](char peek) { return token(peek) && peek != '{'; });
                    std::string var2(r.first, r.second);
                    p.read_while(ws);
                    p.eat('{');
                    p.eat('{');

                    auto context = p.save();
                    auto& vec = ctx[var1];
                    for (object v: dic[var2]) {
                        vec.push_back(v);
                        block(p, dic, ctx);
                        vec.pop_back();
                        p.load(context);
                    }
                    block(p, dic, ctx, false);
                    p.eat('}');
                    p.eat('}');
                } else if (command == "if") {
                    p.read_while(ws);
                    r = p.read_while([token](char peek) { return token(peek) && peek != '{'; });
                    std::string var(r.first, r.second);
                    p.read_while(ws);
                    p.eat('{');
                    p.eat('{');
                    bool run = static_cast<bool>(dic[var]);
                    block(p, dic, ctx, run);
                    while (true) {
                        p.eat('}');
                        p.eat('}');
                        auto context = p.save();
                        p.read_while(ws);
                        c = p.peek();
                        if (c == '$') {
                            p.read();
                            r = p.read_while([token](char peek) { return token(peek) && peek != '{'; });
                            std::string command(r.first, r.second);
                            if (command == "elseif") {
                                p.read_while(ws);
                                r = p.read_while([token](char peek) { return token(peek) && peek != '{'; });
                                std::string var(r.first, r.second);
                                p.read_while(ws);
                                p.eat('{');
                                p.eat('{');
                                bool run_ = static_cast<bool>(dic[var]);
                                block(p, dic, ctx, not run && run_);
                                if (not run && run_)
                                    run = true;
                            } else if (command == "else") {
                                p.read_while(ws);
                                p.eat('{');
                                p.eat('{');
                                block(p, dic, ctx, not run);
                                p.eat('}');
                                p.eat('}');
                                break;
                            } else {
                                p.load(context);
                                break;
                            }
                        } else {
                            break;
                        }
                    }
                } else {
                    throw __LINE__;
                }
            }
        }
    }
}

void parse(std::string input, tmpl& t) {
    parser<std::string::iterator> p{input.begin(), input.end()};
    tmpl_context ctx;
    block(p, t, ctx);
}

}

int main() {
    ginger::tmpl t;
    t["xs"] = std::vector<int>{ 1, 2, 3, 4 };
    t["value"] = 100;
    t["x"] = true;
    t["p"] = false;
    t["q"] = false;
    std::string input = getStdin();
    try {
        ginger::parse(input, t);
    } catch (int line) {
        std::cerr << "error: " << line << std::endl;
    }
}
