
# Documentation


# Example script

The following example will create a fake data inside the tested AMC13 card and then it will
read all events and check their number (we are expecting to get back dynamically computed
range of values).

This file has to be executed via the `amct` tool, it is a syntactically correct Python code,
but the `amc` object is available only if the code is executed correctly.

```
amc.run("wv 100") # write random data
amc.check("rd ...", amc.compute_wordsize(100)) # check that we read correct number of words
```    



# Variables for extrapolation

Before the script gets executed, certain variables are made available.