# GraphRunner Coding Conventions

This project adopts the official Rust API Guidelines as its primary coding convention to maintain clean, readable, and consistent code.

## 1\. Naming Conventions

일관된 이름은 코드의 의도를 명확하게 전달하는 첫걸음입니다.

### General Rules

  - **Use `PascalCase` for type-level constructs.**

      - `class`, `struct`, `enum`, `union`, `typedef`, `using`
      - *Example:* `class RenderPass;`, `struct VertexData;`

  - **Use `snake_case` for value-level constructs.**

      - Variables, function parameters, struct/class members
      - *Example:* `uint32_t frame_index;`, `void set_render_target(...)`

  - **Use `SCREAMING_SNAKE_CASE` for constants and static variables.**

      - `const`, `constexpr`, `static` variables
      - *Example:* `const int MAX_FRAMES_IN_FLIGHT = 2;`

  - **Use `PascalCase` for namespaces.** Namespaces act like modules or crates in Rust.

      - *Example:* `namespace GraphRunner { ... }`, `namespace RHI { ... }`

  - **Prefix interfaces with `I`.** While not a strict Rust convention, it's a common and useful practice in C++ for abstract classes.

      - *Example:* `class IRenderer;`

  - **Prefix private/protected member variables with `m_`.** This clearly distinguishes member variables from local variables.

      - *Example:* `uint32_t m_frame_index;`

### Naming Summary

| Construct | Case | Example |
| :--- | :--- | :--- |
| `class`, `struct`, `enum` | `PascalCase` | `class MeshRunner;` |
| Functions / Methods | `snake_case` | `void process_input();` |
| Variables / Parameters | `snake_case` | `int buffer_size;` |
| Member Variables | `m_snake_case` | `int m_buffer_size;` |
| `namespace` | `PascalCase` | `namespace Core;` |
| Constants (`const`/`static`) | `SCREAMING_SNAKE_CASE` | `const int MAX_LIGHTS = 8;`|
| Interfaces (Abstract Class)| `IPascalCase` | `class IWindow;` |

-----

## 2\. Formatting

일관된 포맷팅은 시각적인 편안함과 가독성을 높입니다.

### Indentation

  - **Use 4 spaces for indentation, not tabs.**
      - `rustfmt`의 기본 설정이며, 가장 널리 사용되는 방식입니다.

### Line Length

  - **Keep lines under 80 characters.**
      - 가독성을 위해 한 줄에 너무 많은 코드를 담지 않도록 합니다. 긴 줄은 여러 줄로 나눕니다.

### Braces

  - **Place opening braces on the same line as the declaration, separated by a space.** (Same-line braces)

      - Rust의 표준 스타일입니다.

    <!-- end list -->

    ```cpp
    // Good
    namespace GraphRunner {
    class MyClass {
        void do_something() {
            if (condition) {
                // ...
            }
        }
    };
    }

    // Bad
    namespace GraphRunner
    {
    class MyClass
    {
        void do_something()
        {
            if (condition)
            {
                // ...
            }
        }
    };
    }
    ```

### Control Flow

  - **Always use braces for control flow statements, even for single lines.**

      - 이는 잠재적인 버그를 방지합니다.

    <!-- end list -->

    ```cpp
    // Good
    if (condition) {
        do_something();
    }

    // Bad
    if (condition) do_something();
    ```

-----

## 3\. Comments

주석은 코드가 "왜" 이렇게 작성되었는지를 설명해야 합니다.

  - **Use `///` for documentation comments (Doxygen style).**
      - 함수, 클래스, 멤버 변수 등의 공개 API를 설명할 때 사용합니다.
  - **Use `//` for single-line implementation comments.**
      - 코드 블록 내부의 복잡한 로직을 설명할 때 사용합니다. `/* ... */` 형태의 블록 주석은 최소화합니다.

<!-- end list -->

```cpp
/// @brief Represents a single rendering pass in the render graph.
class RenderPass {
public:
    /// @brief Executes the rendering commands for this pass.
    /// @param command_buffer The command buffer to record commands into.
    void execute(CommandBuffer& command_buffer) {
        // A single-line comment explaining a complex part of the implementation.
        // ...
    }

private:
    /// The name of this render pass.
    std::string m_name;
};
```

-----

## Tooling

To enforce these conventions automatically, this project will use **ClangFormat**. The `.clang-format` configuration file in the root directory is based on these Rust-style guidelines.
