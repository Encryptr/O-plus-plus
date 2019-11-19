# Announcement
NEW UPDATE! O++ has temporarily turned into a lisp family language for learning purposes. I have successfully implemented an AST and I'm hoping to expand on that with the old O++ syntax. New updates will be coming soon as I keep slowly working, I am hoping to finish by having a stable O++ Language release and ports for O++ to other platforms and frameworks.

# O++ Language
By: Maks Sawoniewicz

# Purpose

When I started writing this language I wanted to learn more about C and introduce a unique idea. An Interpreter that acts as a compiler but only when it needs to. I didn't want to make this another object-oriented language or a regular language so I once again introduced some interesting ideas. To begin, I decided with calling this language something along the lines of class or memory oriented. This is because language acts sort of like a compiler. The first thing it looks for is a class just like a main() function in C. But also if there is no class you can write regular script code from top to bottom and it will interpret it. If you define a class it allocates memory for all the things in that class meaning its variables and so on. This will allow you to take classes when only needed to make sure you later will need those variables and share them across files with always knowing your variables will have enough memory to expand. To furthermore explain the point of this language I have to explain my beginning goal. Write a language that I can later write another language in! Funny right but true. Because of this idea, I decided to make this language focused on having interesting features when opening files just like in C but with less messy and overcomplicated syntax.

Recently a friend pointed out that the name of this language is the same name as the "universal donor" blood type O. This made me change the idea for o++ quite a bit. I want O++ to expand to be a universal donor or capable of being used for lots of purposes. For this to be achieved I hope people are inspired to create libraries and expand the power of O++ to reach its full potential. 

# Run / Compile
To run O++ go into the src file and run the command "gcc o++.c -o o++" in the terminal. Ignore any warning and then launch the interpreter on any file by typing "./o++ filname" on linux and mac on windows simply type "o++ filename".

# Using O++ Syntax
If you want to use my own custom made o++ syntax highlighting, I made an XML file under the XML folder that you can import to notepad++ to use O++ syntax and grammar.

# Examples

Hello World & Comments:
```ruby
## Comment Example ##
print @var_name
print 'Hello World!'
```
More examples under the "example" folder!
