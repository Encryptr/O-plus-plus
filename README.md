# O++ Language
By: Maks Sawoniewicz

# Purpose

When I started writing this language I wanted to learn more about C and introduce a unique idea. An Interpreter that acts as a compiler but only when it needs to. I didn't want to make this another object-oriented language so I decided with something along the lines of class or memory oriented. The way this acts like this is that if you want to write regular script code you just write from top to bottom and the interpreter will interpret it. If you define a class it will act as if it's seeing int main and allocate memory for all the things in that class meaning its variables and so on. This will allow you to make classes when only needed to make sure you later will need those variables and share them across files never losing the address to them. This once again means that the classes act like functions but where you can define whether the variables are public to all other files or private.

# Examples

Using classes:
```ruby
class 

  @age : 10

end

print @age
```
Variables and Statements:
```ruby
noclass

@a : 2
@b : 6
@c : @a + @b

if @c :: 8
  print "C is 8"

eif @c ! 8
  print "C is not 8"

endif
```
