This HW4 is about high-level code generation

Several tricky parts:

1. to get both array symbol and base/pointer type from one node, I modified the getType() function and sth. else so that one node can have different type and symbol

2. so many const functions in type.h, such as storage calculator, which prevents me to set memory offset for each field of the struct. Therefore, I can only set them again.

3. const get_member() function requires mutable member variable

4. signed converting to unsigned requires no change

5. string values need to be specially dealt with. I created a string storage struct type, and added a vector containing these structs inside LocalStorageAllocation class.
Whenever I reached a node with string literal value, I added a struct to the vector, and created appropriate operand stored in that node.