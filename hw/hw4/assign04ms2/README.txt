This HW4 is about high-level code generation

Several tricky parts:

1. to get both array symbol and base/pointer type from one node, I modified the getType() function and sth. else so that one node can have different type and symbol

2. so many const functions in type.h, such as storage calculator, which prevents me to set memory offset for each field of the struct. Therefore, I can only set them again.

3. const get_member() function requires mutable member variable

4. signed converting to unsigned requires no change

5. string values need to be specially dealt with. I created a string storage struct type, and added a vector containing these structs inside LocalStorageAllocation class.
Whenever I reached a node with string literal value, I added a struct to the vector, and created appropriate operand stored in that node.

6. multi-D array needs special care

7. In my implementation, I need to check both type and symbol of an array variable, but if it inside a struct, then we cannot find that symbol anymore.
We can only find the member. Therefore, to simplify other codes, I created a unique_ptr of symbol, with similar content in that member, and use .get()
to pass it to the node.

8. when 2 memref are in one instruction, only %r10 anf %r11 may not be enough. After discussing with Prof, I used %rax.