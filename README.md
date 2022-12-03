# pysh

[Get the Pysh Syntax Highlighter extension for Visual Studio Code](https://marketplace.visualstudio.com/items?itemName=RahulYedida.pysh-highlighting)

pysh is a syntax that transpiles to Python. It allows you to embed shell scripts into your Python code for syntactic sugar. 

## Syntax

`pysh` has the following major syntax additions to Python:

* **Inline evaluations:** These allow you to write single-line shell commands whose outputs are returned.

Suppose you need to count the number of iterations a program has run in a file, and the program outputs lines starting with `iter i`. You could then do
```
iterations = int`grep "^iter" | wc -l`
```

The `int` *formatter* is a built-in function that specifies that the result of the shell command must be an integer. If this is not satisfied, it raises a `TypeError`.

Another example involves `cut`. To extract the second field from each line of a `|`-delimited file, one could use the `pysh` syntax as follows:

```
favorite_colors = list.str`cut -d '|' -f2 input.txt`
```

Here, we use the `list.str` formatter, since `cut` returns multiple lines of output, that we wish to be formatted into a list, each element of which we want to be a str. One could also use the `list` formatter itself, and the `.str` is implicitly assumed, but the former is a clearer syntax.

* **Inline Python code:** You can use `pysh` to run commands that either return no output, or whose output you wish to ignore.

The following example runs 20 jobs in slurm, waiting 3 seconds after each to ensure there was no error (which would stop the slurm job).

```
import time

for i in range(20):
    output = str`sbatch run.sh`  # output is "Submitted job <job-id>
    job_id = output.split()[-1]

    time.sleep(3)
    job_running = int`squeue | grep {{job_id}} | wc -l` > 0
    if not job_running:
        print("Job", job_id, "is not running!")
```

In this code, we use the double bracket syntax to use a Python variable in a shell command.

* **Embedded shell scripts:** One could also embed multi-line shell scripts as follows:

In this example, we concatenate the output of several files produced by slurm jobs into a list of strings.

    lines = list.str```
    for file in slurm*.out
    do
        cat $file
    done
    ```

