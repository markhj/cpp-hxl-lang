![Project Image](https://res.cloudinary.com/drfztvfdh/image/upload/v1720000791/Github/hxl-for-cpp_dpxqim.jpg)

![GitHub Tag](https://img.shields.io/github/v/tag/markhj/cpp-hxl-lang?label=version)
![License](https://img.shields.io/badge/license-MIT-green.svg)

**HXL for C++** is a library for the **HXL** data interchange
format. Bundled is an all-around solution with:

- Tokenizer
- Parser
- Semantic analyzer
- Transformer
- Schema validator
- Deserializer

In layman's terms it means that this library can translate an HXL file
into ready-to-use structures in C++.

## 🌿 What is HXL?

If you're unfamiliar with HXL, then it's recommended to read here:

[HXL language specification](https://github.com/markhj/hxl-lang)

The short-story version is that HXL is a data interchange format, like JSON
or YAML, but with some extracurricular features such as references
and inheritance, making the language more flexible and useful.

HXL follows a set of strict syntax rules, ensuring data is streamlined,
readable and easy for a computer to parse.

## 💫 Note

> Important: The implementation is currently in alpha, which means it's
> functional, but still not fully covering all rules from the language
> specification.

## 📌 Prerequisites

- C++20 or later
- CMake 3.28 or later

## 🎫 Use in your own project

There are two things to do, when setting HXL for use in your own project:

- Clone and build (outside your project scope)
- Integrate in your own project

The idea is then to instruct your project on where to find the HXL
library, which can differ from machine to machine. This is handled
with environment variables.

### Clone and build HXL

````bash
git clone https://github.com/markhj/cpp-hxl-lang
````

After cloning, you must initialize the submodules:

````bash
git submodule init
git submodule update
````

And now, use CMake to build the project. You should, depending
on the profiles you've set up, see one or both of ``debug`` and
``release`` getting created in the ``build`` directory.

> Important: The mode you're using in your own project must have
> a matching build of HXL.

### Integrate in your project

The next step is to instruct CMake on where it can find the build
you just made. You do so, by declaring an environment variable
for CMake called ``CPP_HXL_LANG_DIR``, which points to the root
of the HXL library directory.

Now, in your ``CMakeLists.txt`` file, you add:

````cmake
find_package(cpp_hxl_lang REQUIRED)
````

And, assuming your executable is called ``MyApp``, you need to
add this part _after_ the ``add_executable`` call.

````cmake
target_link_libraries(MyApp PRIVATE cpp_hxl_lang)
````

## 🚀 Basic usage

The first step is to include the library:

````c++
#include <hxl-lang/hxl-lang.h>
````

Next, we need to define two components:

- **Schema**: Describes the data you expect to see
- **Deserialization protocol**: An instruction set for converting
  the parsed data to structures you can work with.

### Schema

In the schema you declare all the node types you want to support.
If a node type is encountered which isn't in the schema, it will
result in an error.

````c++
Schema schema;

SchemaNodeType nameOfType{"NameOfType"};
material.properties.emplace_back("string_property", DataType::String);
schema.types.push_back(nameOfType);
````

In the above, a node type called ``NameOfType`` is declared, and it has a string
property called ``string_property``.

This means, HXL would now accept a file that looked like this:

````text
<NameOfType> A
    string_property: "Hello
    
<NameOfType> B
    string_property: "World"
````

> Note: Tutorial on other data types will arrive very soon!

### Deserialization protocol

The aptly named "Deserialization Protocol" explains HXL what it should
do when it encounters a node type. Imagine, with the above, that
we want to put the information into a struct called ``MyAwesomeStruct``.

````c++
struct MyAwesomeStruct {
    std::string name, string_property;
};

DeserializationProtocol protocol;

DeserializationHandle nameOfTypeHandle{"NameOfType"};
material.handle = [&](const DeserializedNode &node) {
    // Do what you need to do :-)
    // In many use cases, you'll just want to create
    // instances of ``MyAwesomeStruct`` and push them onto a vector,
    // or something to that effect
    
    std::cout << node.name << " has " << node.properties.size() << " properties";
};
protocol.handles.push_back(nameOfTypeHandle);
````

> 💡 **Tip:** You can explore the samples to see how you can use these
> features to create flexible applications.

### Process a file

Now, you're ready to start processing HXL sources. You can either
manually control the translation flow, or use the built-in ``HXL::Processor``.

````c++
// hxlSource is probably loaded from a file, but it can come
// from anywhere, as long as it's passed as a string

ProcessResult result = Processor::process(hxlSource, schema, protocol);
````

At this point, if no errors were encountered, the handled defined
in the deserialization protocol will have been fired.

However, if errors are encountered, you need to handle them. For example,
by displaying the first one (you can, of course, also show all, but it might
cause unnecessary confusion):

````c++
if (!result.errors.empty()) {
    std::cerr << result.errors[0].message << std::endl;
}
````

And that's it! Now you can parse HXL files.

## 🚧 Exploring or expanding the library

To work on the repository itself locally, you start by cloning:

````bash
git clone https://github.com/markhj/cpp-hxl-lang
````

After cloning, you must initialize the submodules:

````bash
git submodule init
git submodule update
````

And now, you're ready!

However, to compile something other than just the library, you
might want to set one or both of these environment variables to
``1``.

| Variable | Function                                            |
| --- |-----------------------------------------------------|
| ``HXL_BUILD_SAMPLES`` | Builds all the samples in the ``samples`` directory |
| ``HXL_BUILD_TESTS`` | Builds the test suite                               |

