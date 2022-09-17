# Team Red GameEngine
## How to build
1. You'll need to download the latest version of CMake https://cmake.org/download/
    (Make sure you add cmake to your PATH)
2. Open console and navigate to the root directory of the project.
3. Run `cmake .` This will configure the CMake files. For more options https://cmake.org/cmake/help/latest/manual/cmake.1.html#generate-a-project-buildsystem
4. Run `cmake --build .` to build the project

The project should now be built according to the configure used in step 3.

# Style Guidelines
Style guidelines are meant to keep the code readable and cosistent for all members in the project. These guideliness are based off https://github.com/cpp-best-practices/cppbestpractices/blob/master/03-Style.md

## Naming Conventions
 * prefer `CamelCase` above `snake_case`.
 * Types start with upper case: `MyClass`.
 * Functions and variables start with lower case: `myMethod`.
 * Constants are all upper case: `const double PI=3.14159265358979323;`.
 * Macro names use upper case with underscores: `INT_MAX`.
 * Template parameter names use camel case: `InputIterator`.
 * All other names use snake case: `unordered_map`.
 
 ## Distinguish Private Object Data

Name private data with a `m_` prefix to distinguish it from public data. `m_` stands for "member" data.

 ## Distinguish Entities and Components
 Variables, types, and memebers of these three types should be suffixed with the corresponding types.
```cpp
 struct TransformComponent
 {
    //...
 };
 
 //...
 
 Entity characterEntity = Entity();
 
 //...
 
 private void renderingSystem()
 {
    //...
 }
```

## Don't Name Anything Starting With `_`

If you do, you risk colliding with names reserved for compiler and standard library implementation use:

http://stackoverflow.com/questions/228783/what-are-the-rules-about-using-an-underscore-in-a-c-identifier


## Well-Formed Example

```cpp
class MyClass
{
public:
  MyClass(int t_data)
    : m_data(t_data)
  {
  }

  int getData() const
  {
    return m_data;
  }

private:
  int m_data;
};
```

## Use `nullptr`

C++11 introduces `nullptr` which is a special value denoting a null pointer. This should be used instead of `0` or `NULL` to indicate a null pointer.

## Comments

Comment blocks should use `//`, not `/* */`. Using `//` makes it much easier to comment out a block of code while debugging.

```cpp
// this function does something
int myFunc()
{
}
```

To comment out this function block during debugging we might do:

```cpp
/*
// this function does something
int myFunc()
{
}
*/
```

which would be impossible if the function comment header used `/* */`.

## Never Use `using namespace` in a Header File

This causes the namespace you are `using` to be pulled into the namespace of all files that include the header file.
It pollutes the namespace and it may lead to name collisions in the future.
Writing `using namespace` in an implementation file is fine though.


## {} Are Required for Blocks.
Leaving them off can lead to semantic errors in the code.

```cpp
// Don't
for (int i = 0; i < 15; ++i)
  std::cout << i << std::endl;

// Don't
int sum = 0;
for (int i = 0; i < 15; ++i)
  ++sum;
  std::cout << i << std::endl;


// Do
// It's clear which statements are part of the loop (or if block, or whatever).
int sum = 0;
for (int i = 0; i < 15; ++i) 
{
  ++sum;
  std::cout << i << std::endl;
}
```

## Do use new lines on {}.

```cpp
// Don't
private void foo() {
  std::cout << "bad!" << std::endl;
}

// Do
private void foo() 
{
  std::cout << "better!" << std::endl;
}
```

## Keep Lines a Reasonable Length

```cpp
// Bad Idea
// hard to follow
if (x && y && myFunctionThatReturnsBool() && caseNumber3 && (15 > 12 || 2 < 3)) {
}

// Good Idea
// Logical grouping, easier to read
if (x && y && myFunctionThatReturnsBool()
    && caseNumber3
    && (15 > 12 || 2 < 3)) {
}
```

Many projects and coding standards have a soft guideline that one should try to use less than about 80 or 100 characters per line.
Such code is generally easier to read.
It also makes it possible to have two separate files next to each other on one screen without having a tiny font.

## Initialize Member Variables
...with the member initializer list.

For POD types, the performance of an initializer list is the same as manual initialization, but for other types there is a clear performance gain, see below.

```cpp
// Bad Idea
class MyClass
{
public:
  MyClass(int t_value)
  {
    m_value = t_value;
  }

private:
  int m_value;
};

// Bad Idea
// This leads to an additional constructor call for m_myOtherClass
// before the assignment.
class MyClass
{
public:
  MyClass(MyOtherClass t_myOtherClass)
  {
    m_myOtherClass = t_myOtherClass;
  }

private:
  MyOtherClass m_myOtherClass;
};

// Good Idea
// There is no performance gain here but the code is cleaner.
class MyClass
{
public:
  MyClass(int t_value)
    : m_value(t_value)
  {
  }

private:
  int m_value;
};

// Good Idea
// The default constructor for m_myOtherClass is never called here, so
// there is a performance gain if MyOtherClass is not is_trivially_default_constructible.
class MyClass
{
public:
  MyClass(MyOtherClass t_myOtherClass)
    : m_myOtherClass(t_myOtherClass)
  {
  }

private:
  MyOtherClass m_myOtherClass;
};
```

In C++11 you can assign default values to each member (using `=` or using `{}`).

### Assigning default values with =

```cpp
// ... //
private:
  int m_value = 0; // allowed
  unsigned m_value_2 = -1; // narrowing from signed to unsigned allowed
// ... //
```
This ensures that no constructor ever "forgets" to initialize a member object.

### Assigning default values with brace initialization

Using brace initialization does not allow narrowing at compile-time.

```cpp
// Best Idea

// ... //
private:
  int m_value{ 0 }; // allowed
  unsigned m_value_2 { -1 }; // narrowing from signed to unsigned not allowed, leads to a compile time error
// ... //
```

Prefer `{}` initialization over `=` unless you have a strong reason not to.

Forgetting to initialize a member is a source of undefined behavior bugs which are often extremely hard to find.

If the member variable is not expected to change after the initialization, then mark it `const`.

```cpp
class MyClass
{
public:
  MyClass(int t_value)
    : m_value{t_value}
  {
  }

private:
  const int m_value{0};
};
```

Since a const member variable cannot be assigned a new value, such a class may not have a meaningful copy assignment operator.

## Always Use Namespaces

There is almost never a reason to declare an identifier in the global namespace. Instead, functions and classes should exist in an appropriately named namespace or in a class inside of a namespace. Identifiers which are placed in the global namespace risk conflicting with identifiers from other libraries (mostly C, which doesn't have namespaces).


## Use the Correct Integer Type for Standard Library Features

The standard library generally uses `std::size_t` for anything related to size. The size of `size_t` is implementation defined.

In general, using `auto` will avoid most of these issues, but not all.

Make sure you stick with the correct integer types and remain consistent with the C++ standard library. It might not warn on the platform you are currently using, but it probably will when you change platforms.

*Note that you can cause integer underflow when performing some operations on unsigned values. For example:*

```cpp
std::vector<int> v1{2,3,4,5,6,7,8,9};
std::vector<int> v2{9,8,7,6,5,4,3,2,1};
const auto s1 = v1.size();
const auto s2 = v2.size();
const auto diff = s1 - s2; // diff underflows to a very large number
```

## Use .h and .cpp for Your File Extensions

Ultimately this is a matter of preference, but .h and .cpp are widely recognized by various editors and tools. So the choice is pragmatic. Specifically, Visual Studio only automatically recognizes .cpp and .cxx for C++ files, and Vim doesn't necessarily recognize .cc as a C++ file.

One particularly large project ([OpenStudio](https://github.com/NREL/OpenStudio)) uses .h and .cpp for user-generated files and .hxx and .cxx for tool-generated files. Both are well recognized and having the distinction is helpful.

## Never Mix Tabs and Spaces

Some editors like to indent with a mixture of tabs and spaces by default. This makes the code unreadable to anyone not using the exact same tab indentation settings. Configure your editor so this does not happen.

## Never Put Code with Side Effects Inside an assert()

```cpp
assert(registerSomeThing()); // make sure that registerSomeThing() returns true
```

The above code succeeds when making a debug build, but gets removed by the compiler when making a release build, giving you different behavior between debug and release builds.
This is because `assert()` is a macro which expands to nothing in release mode.

## Don't Be Afraid of Templates

They can help you stick to [DRY principles](http://en.wikipedia.org/wiki/Don%27t_repeat_yourself).
They should be preferred to macros, because macros do not honor namespaces, etc.

## Use Operator Overloads Judiciously

Operator overloading was invented to enable expressive syntax. Expressive in the sense that adding two big integers looks like `a + b` and not `a.add(b)`. Another common example is `std::string`, where it is very common to concatenate two strings with `string1 + string2`.

However, you can easily create unreadable expressions using too much or wrong operator overloading. When overloading operators, there are three basic rules to follow as described [on stackoverflow](http://stackoverflow.com/questions/4421706/operator-overloading/4421708#4421708).

Specifically, you should keep these things in mind:

* Overloading `operator=()` when handling resources is a must. See [Consider the Rule of Zero](03-Style.md#consider-the-rule-of-zero) below.
* For all other operators, only overload them when they are used in a context that is commonly connected to these operators. Typical scenarios are concatenating things with +, negating expressions that can be considered "true" or "false", etc.
* Always be aware of the [operator precedence](http://en.cppreference.com/w/cpp/language/operator_precedence) and try to circumvent unintuitive constructs.
* Do not overload exotic operators such as ~ or % unless implementing a numeric type or following a well recognized syntax in specific domain.
* [Never](http://stackoverflow.com/questions/5602112/when-to-overload-the-comma-operator?answertab=votes#tab-top) overload `operator,()` (the comma operator).
* Use non-member functions `operator>>()` and `operator<<()` when dealing with streams. For example, you can overload `operator<<(std::ostream &, MyClass const &)` to enable "writing" your class into a stream, such as `std::cout` or an `std::fstream` or `std::stringstream`. The latter is often used to create a string representation of a value.
* There are more common operators to overload [described here](http://stackoverflow.com/questions/4421706/operator-overloading?answertab=votes#tab-top).

More tips regarding the implementation details of your custom operators can be found [here](http://courses.cms.caltech.edu/cs11/material/cpp/donnie/cpp-ops.html).

## Avoid Implicit Conversions

### Single Parameter Constructors

Single parameter constructors can be applied at compile time to automatically convert between types. This is handy for things like `std::string(const char *)` but should be avoided in general because they can add to accidental runtime overhead.

Instead mark single parameter constructors as `explicit`, which requires them to be explicitly called.

### Conversion Operators

Similarly to single parameter constructors, conversion operators can be called by the compiler and introduce unexpected overhead. They should also be marked as `explicit`.

```cpp
//bad idea
struct S {
  operator int() {
    return 2;
  }
};
```

```cpp
//good idea
struct S {
  explicit operator int() {
    return 2;
  }
};
```

## Consider the Rule of Zero

The Rule of Zero states that you do not provide any of the functions that the compiler can provide (copy constructor, copy assignment operator, move constructor, move assignment operator, destructor) unless the class you are constructing does some novel form of ownership.

The goal is to let the compiler provide optimal versions that are automatically maintained when more member variables are added.
