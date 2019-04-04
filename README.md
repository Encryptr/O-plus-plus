# O++ Language
By: Maks Sawoniewicz

# Purpose

When I started writing this language I wanted to learn more about C and introduce a unique idea. An Interpreter that acts as a compiler but only when it needs to. I didn't want to make this another object-oriented language so I decided with something along the lines of class or memory oriented. This language acts like if you want to write regular script code you just write from top to bottom and the interpreter will interpret it. If you define a class it will act as if it's seeing int main and allocate memory for all the things in that class meaning its variables and so on. This will allow you to make classes when only needed to make sure you later will need those variables and share them across files never losing the address to them. This once again means that the classes act similar to  functions but where you can define whether the variables are public to all other files or private. And if you dont have a class you can also define noclass to tell the interpreter.

# Using O++ Syntax
If you want to use my own custom made o++ syntax highliting, I made an XML file under the XML folder that you can import to notepad++ to use O++ syntax and grammer.

# Examples

Hello World & Comments:
```ruby
< Comment Example >
< Use this for variables >
print @
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
