# hw5 report

|||
|-:|:-|
|Name|邱振源|
|ID|111550100|

## How much time did you spend on this project

> e.g. 15 hours.

## Project overview

First, I wrote the assembly code form for the visited nodes according to the spec, and then dumped the assembly code based on the necessary situations.

Take unary operation for example:

We can note that evey unary operation should dump 
```
lw t0, 0(sp)
addi sp, sp, 4;
```
and
```
addi sp, sp, -4
sw t0, 0(sp)
```
,and the negative one shloud dump ```neg t0, t0```. Therefore, we can get the following code:
```
void CodeGenerator::visit(UnaryOperatorNode &p_un_op)
{
    cur_root.push_back("Un_op");
	p_un_op.visitChildNodes(*this);
	cur_root.pop_back();
    constexpr const char *const riscv_assembly_unop_s = "    lw t0, 0(sp)\n"
                                                        "    addi sp, sp, 4\n";
    constexpr const char *const riscv_assembly_unop_e = "    addi sp, sp, -4\n"
                                                        "    sw t0, 0(sp)\n";

    dumpInstructions(m_output_file.get(), riscv_assembly_unop_s);
    switch(p_un_op.getOp())
    {
        case Operator::kNegOp:
        {
            constexpr const char *const riscv_assembly_unop = "    neg t0, t0\n";
            dumpInstructions(m_output_file.get(), riscv_assembly_unop);
            dumpInstructions(m_output_file.get(), riscv_assembly_unop_e);
            break;
        }
        case Operator::kNotOp:
        {
            constexpr const char *const riscv_assembly_unop = "    li t1, -1\n"
					                        "    add t0, t0, t1\n";
            dumpInstructions(m_output_file.get(), riscv_assembly_unop);
            dumpInstructions(m_output_file.get(), riscv_assembly_unop_e);
            break;
        }
    }
}
```

## What is the hardest you think in this project

I believe the most challenging part of this assignment was handling the potential issue of having more than eight function arguments. Initially, I had no idea where to store them, but later I thought of storing the values in registers like t0, t1, etc., which successfully solved the problem.

## Feedback to T.A.s

Thank you to the TA for writing a very detailed spec, which provided us with a clear direction to follow. Although I was initially unfamiliar with assembly code and found debugging more difficult, I was able to complete the assignment smoothly by following the spec.
