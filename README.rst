======
ginger
======

Template is here::

  <html>
    <head>
      <title>${title}</title>
    </head>
    <body>
      <p>List of values:</p>
      <ul>
        $for x in xs {{
          $if x.enable {{
            <li><a href="${x.url}">${x.value}</a></li>
          }}
        }}
      </ul>
    </body>
  </html>

Source code is here::

  #include "ginger.h"
  #include <iostream>
  #include <ios>
  #include <vector>
  #include <map>
  #include <string>

  std::string getStdin() {
      std::string output;
      std::string input;
      std::cin >> std::noskipws;
      while (std::getline(std::cin, input)) {
          output += input + '\n';
      }
      return output;
  }

  int main() {
      ginger::temple t;
      std::vector<std::map<std::string, ginger::object>> xs;
      xs.push_back({
          { "enable", true },
          { "url", "http://example.com" },
          { "value", "Example" },
      });
      xs.push_back({
          { "enable", false },
          { "url", "undefined" },
          { "value", "Test" },
      });
      xs.push_back({
          { "enable", true },
          { "url", "http://google.com" },
          { "value", "Google" },
      });
      t["title"] = "Sample Site";
      t["xs"] = xs;

      std::string input = getStdin();
      try {
          ginger::parse(input, t);
      } catch (ginger::parse_error& error) {
          std::cerr << error.long_error() << std::endl;
      }
  }

Result::

  <html>
    <head>
      <title>Sample Site</title>
    </head>
    <body>
      <p>List of values:</p>
      <ul>
        
          
            <li><a href="http://example.com">Example</a></li>
          
        
          
        
          
            <li><a href="http://google.com">Google</a></li>
          
        
      </ul>
    </body>
  </html>

Requirements
============

Input
-----

- below expression should be valid::

    auto first = std::begin(input);
    auto last = std::end(input);

- first, last should be ForwardIterator.


Dictionary
----------

- Below expressions should be valid::

    std::string var;
    auto it = dic.find(var);
    it != dic.end();

- Type of it->second should be object.

Output
------

- Below expression should be valid::

    // Output output; // default constructible does not require.
    // For any ForwardIterator first, last
    output.put(first, last);
    output.flush();

Reference
=========

object
------

Class ``object`` can construct from any value.

::

  class object {
  public:
      object() = default;
      object(const object&) = default;
      object(object&&) = default;
      object& operator=(const object&) = default;
      object& operator=(object&&) = default;

      template<class T> object(T v);
      template<class T> void operator=(T v);
  };

parse
-----

::

  template<class Input, class Dictionary>
  void parse(Input&& input, Dictionary&& t);
  template<class Input, class Dictionary, class Output>
  void parse(Input&& input, Dictionary&& t, Output&& out);

  template<class Dictionary>
  void parse(const char* input, Dictionary&& t);
  template<class Dictionary, class Output>
  void parse(const char* input, Dictionary&& t, Output&& out);

Template Syntax Specification
=============================

::

  <root> = <block>
  <block> = (<char> | <$comment> | <$for> | <$if> | <$variable> | $$ | ${{ | $}}) <block> | <eof>
  <char> = any character without '$'
  <$comment> = $#<comment-char>*
  <comment-char> = any character without '\n' and <eof>
  <$for> = $for <var-name> in <var> {{ <block> }}
  <$if> = $if <var> {{ <block> }} ($elseif <var> {{ <block> }})? ($else {{ <block> }})?
  <$variable> = ${<var>}
  <var> = <var-name>(.<var-name>)*
  <var-name> = <var-char>+
  <var-char> = any character without <whitespace>, '.', '{' or '}'
  <whitespace> = c <= 32 where c is character
