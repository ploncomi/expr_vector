# expr_vector
Class for using operations based on expression templates on std::vector, or on a provided external buffer.

It is provided inside a single header file.

It aims to be similar to std::valarray, but the option for providing an external buffer makes the computations faster, as initialization doesn't require to copy all of the elements.
