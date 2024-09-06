"""
Key idea: gem5 Interprets Python

The gem5 simulator can be thought of as a C++ program that interprets a Python script which defines the simulation. 
gem5 is a Python interpreter that includes the gem5 python libraries.

Python is used as a high level "control language" -- 
essentially a language used to set up and control a bunch of components written with C++.
"""

def my_generator():
    yield 1
    yield 2
    yield 3

for value in my_generator():
    print(value)

# lots of notes on simobject inheritance and immutability.