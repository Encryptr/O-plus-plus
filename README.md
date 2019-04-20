# O++ Language
By: Maks Sawoniewicz

# Purpose

When I started writing this language I wanted to learn more about C and introduce a unique idea. An Interpreter that acts as a compiler but only when it needs to. I didn't want to make this another object-oriented language or a regular language so I once again introduced some interesting ideas. To begin, I decided with calling this language something along the lines of class or memory oriented. This is because language acts sort of like a compiler. First thing it looks for is a class just like a main() function in C. But also if there is no class you can write regular script code from top to bottom and it will interpret it. If you define a class it allocate memory for all the things in that class meaning its variables and so on. This will allow you to make classes when only needed to make sure you later will need those variables and share them across files with always knowing your variables will have enough memory to expand. To furthermore explain the point of this language I have to explain my beginning goal. Write a language that I can later write another language in! Funny right but true. Because of this idea I decided to make this language focused on having interesting features when opening files just like in C but with less messy and over complicated syntax.

# Using O++ Syntax
If you want to use my own custom made o++ syntax highlighting, I made an XML file under the XML folder that you can import to notepad++ to use O++ syntax and grammar.

# Examples

Hello World & Comments:
```ruby
< Comment Example >
print @var_name
< For Quick Printing Strings >
print 'Hello World!'
```
Using classes:
```ruby
class

  @age : 10

end

print @age
```
Variables and Statements:
```ruby
class

@c : 10

end

if @c :: 8
  print "C is 8"

eif @c ! 8
  print "C is not 8"

endif
```
