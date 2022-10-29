This HW3 is about semantic analysis of a nearly c language

I think the two trick parts are handling literal values and arrays

1. As we need to compare types between operands, and operands may be variables or literal values, it is better to give those nodes with literal value an appropriate type,
    so that the comparison could be much simplified.

2. The array name itself could appear in function call arguments, right values, as it should be considered as a pointer to the base type the array contains;
    while at the same time, the array element reference could appear in function parameters, left values, as it should be just the base type of the array

3. My codes could still be simplified, as the first idea was implemented almost at the end, which means that there might be some redundant codes.
