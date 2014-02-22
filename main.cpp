#include <iostream>
#include <ios>
#include <string>
#include <utility>
#include <map>
#include <vector>

std::string getStdin() {
    std::string output;
    std::string input;
    std::cin >> std::noskipws;
    while (std::getline(std::cin, input)) {
        output += input + '\n';
    }
    return output;
}

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
            throw 0;
        }
        virtual bool cond() const override {
            return cond_<T>(0);
        }

        template<class U, std::size_t = sizeof(std::declval<U>().begin(), std::declval<U>().end())>
        iterator begin_(short) {
            return obj.begin();
        }
        template<class U, std::size_t = sizeof(begin(std::declval<U>()), end(std::declval<U>()))>
        iterator begin_(int) {
            return begin(obj);
        }
        template<class U, std::size_t = sizeof(std::begin(std::declval<U>()), std::end(std::declval<U>()))>
        iterator begin_(long) {
            return std::begin(obj);
        }
        template<class>
        iterator begin_(...) {
            throw 0;
        }
        virtual iterator xbegin() override {
            return begin_<T>(0);
        };

        template<class U, std::size_t = sizeof(std::declval<U>().begin(), std::declval<U>().end())>
        iterator end_(short) {
            return obj.end();
        }
        template<class U, std::size_t = sizeof(begin(std::declval<U>()), end(std::declval<U>()))>
        iterator end_(int) {
            return end(obj);
        }
        template<class U, std::size_t = sizeof(std::begin(std::declval<U>()), std::end(std::declval<U>()))>
        iterator end_(long) {
            return std::end(obj);
        }
        template<class>
        iterator end_(...) {
            throw 0;
        }
        virtual iterator xend() override {
            return end_<T>(0);
        }

        template<class U, int = sizeof(std::to_string(std::declval<U>()))>
        std::string str_(int) const {
            return std::to_string(obj);
        }
        template<class>
        std::string str_(...) const {
            throw 0;
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

class parser {
public:
    std::string input;
    int index;
    parser(std::string input) : input(std::move(input)), index(0) { }
    void read() {
        if (index >= input.length()) throw 0;
        input[index++];
    }
    char peek() const {
        if (index >= input.length()) throw 0;
        return input[index];
    }
    char next() const {
        if (index + 1 >= input.length()) throw 0;
        return input[index + 1];
    }
};

void block(parser& p) {
    char c = p.peek();
    if (c != '$') {
        std::cout << c;
        p.read();
    } else {
        p.read();
        char d = p.peek();
        if (d == '$') {
            p.read();
            std::cout << d;
        } else if (d == '#') {
            p.read();
            while (p.peek() != '\n')
                p.read();
        } else if (d == '{') {
            p.read();
            d = p.peek();
            if (d == '{') {
                p.read();
                std::cout << "{{";
            } else {
                std::cout << "[variable(";
                do {
                    std::cout << d;
                    p.read();
                    d = p.peek();
                } while (d != '}' && d != ' ');
                p.read();
                std::cout << ")]";
            }
        } else if (d == '}') {
            p.read();
            d = p.peek();
            if (d == '}') {
                p.read();
                std::cout << "}}";
            } else {
                throw 0;
            }
        } else {
            std::string name;
            do {
                name.push_back(d);
                p.read();
                d = p.peek();
            } while (d != ' ');
            if (name == "for") {
                std::cout << "$for block";
            } else if (name == "if") {
                std::cout << "$if block";
            } else if (name == "elseif") {
                std::cout << "$elseif block";
            } else if (name == "else") {
                std::cout << "$else block";
            }
        }
    }
    block(p);
}

void parse(std::string input) {
    try {
        parser p{std::move(input)};
        block(p);
    } catch (...) {
        std::cout << std::endl;
    }
}

int main() {
    tmpl t;
    t["test"] = std::vector<int>{ 1, 2, 3, 4 };
    for (object v: t["test"]) {
        std::cout << v.str() << std::endl;
    }
    std::string input = getStdin();
    parse(input);
}
