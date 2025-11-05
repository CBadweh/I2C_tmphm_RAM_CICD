# How to Write Prompts for Clear Technical Explanations

## What Worked Well

When you asked: **"Can we use both I2C_INSTANCE_3 and I2C_NUM_INSTANCES interchangeably?"**

This triggered a response that:
- ✅ Showed actual values (= 0, = 1)
- ✅ Explained different purposes
- ✅ Provided concrete code examples for each scenario
- ✅ Showed what happens if you do it wrong (out of bounds)

---

## Effective Prompt Patterns

### 1. **Ask "Where and How" Questions**

**Good:**
- "Where and how is `X` used in the code?"
- "How does `X` differ from `Y` in actual usage?"

**Why it works:** Forces specific examples showing context and usage patterns.

**Example:**
```
"Where and how is I2C_NUM_INSTANCES used in i2c.c?"
```

---

### 2. **Ask About Interchangeability**

**Good:**
- "Can I use `X` and `Y` interchangeably?"
- "What's the difference between `X` and `Y` in practice?"
- "Are `X` and `Y` the same thing?"

**Why it works:** Triggers comparison with examples showing when to use each.

**Example:**
```
"Can I use I2C_INSTANCE_3 and I2C_NUM_INSTANCES interchangeably?"
```

---

### 3. **Ask "What Happens If..." Questions**

**Good:**
- "What happens if I use `X` instead of `Y` here?"
- "What breaks if I change this value?"
- "What's wrong with doing it this way: [code]"

**Why it works:** Gets you concrete examples of failure cases and edge cases.

**Example:**
```
"What happens if I use i2c_states[I2C_NUM_INSTANCES]?"
```

---

### 4. **Request Concrete Examples**

**Good:**
- "Show me examples of how `X` is used in different scenarios"
- "Give me code examples for each use case"
- "Show actual values, not just descriptions"

**Why it works:** Forces practical demonstrations instead of abstract explanations.

**Example:**
```
"Show me examples of where I should use I2C_INSTANCE_3 vs I2C_NUM_INSTANCES"
```

---

### 5. **Ask About Purpose/Intent**

**Good:**
- "What's the purpose of `X`?"
- "Why does the code use `X` here instead of `Y`?"
- "What problem does this solve?"

**Why it works:** Gets underlying reasoning with examples showing why design choices matter.

**Example:**
```
"Why does the enum have I2C_NUM_INSTANCES at the end?"
```

---

## Prompt Templates

### Template 1: Clarify Similar-Looking Things
```
"What's the difference between [A] and [B]? 
Can they be used interchangeably? 
Show me examples of when to use each."
```

### Template 2: Understand Usage Patterns
```
"Where and how is [X] used in [file/module]?
Show me concrete examples for different scenarios."
```

### Template 3: Expose Actual Values
```
"What are the actual values of [X], [Y], and [Z]?
How do these values affect how they're used in code?"
```

### Template 4: Learn from Mistakes
```
"What happens if I use [X] like this: [code snippet]?
What would break and why?"
```

### Template 5: Compare Approaches
```
"How does [approach A] differ from [approach B]?
Show me code examples of both and when to use each."
```

---

## What to Avoid

❌ **Vague questions:**
- "Tell me about enums" (too broad)
- "Explain this" (no specific focus)

❌ **Yes/no questions without follow-up:**
- "Is this correct?" (doesn't prompt detailed explanation)

❌ **Abstract-only questions:**
- "What is the concept behind this?" (may skip practical examples)

---

## Power Combo: Stack Multiple Techniques

**Example:**
```
"What's the difference between I2C_INSTANCE_3 and I2C_NUM_INSTANCES?
Show me their actual values.
Where is each one used in i2c.c?
Can they be used interchangeably?
What breaks if I use the wrong one?"
```

This stacks:
- Comparison request
- Value request
- Usage pattern request
- Interchangeability check
- Failure case exploration

---

## Key Insight

**Ask questions that force specific, concrete examples rather than abstract explanations.**

Instead of "Explain X" → Ask "Show me how X is used differently from Y in actual code"

Instead of "What is X?" → Ask "What's the actual value of X and how does that affect its usage?"

Instead of "How does X work?" → Ask "Where and how is X used? What happens if I do it wrong?"

