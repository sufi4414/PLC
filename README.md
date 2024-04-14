# PLC

## Quick usage guide

Run `make` in the directory. Then run `./text2wav {input}.txt`. The 3 premade `input` files are `apple.txt`, `ghosts.txt` and `future.txt`. After running the command, you can listen to the outputted `output.wav`.

## Introduction

In this project, we will turn a coding-like text into a set of notes to be played, which will in turn turn it into wav audio. 

### Details of the audio:

We will directly write the sound wave shapes into the wav file data. We have 5 types of waves: sin waves, triangle waves, saw waves, square waves, and a kick wave which is supposed to represent a drum. Range of notes is according to https://muted.io/note-frequencies/. We got a reference on how to write the wav header and the samples from https://kaizen.place/music-tails/writing-wav-files-in-c-630150ea1b3ccbc86a636770

### Intermediary language:
We will use an intermediary language which is a csv that specifies which notes to play at which timings. The format of the csv is given here with some example entries:

| Frequency (Hz) | Start time (float)| End time (float)| Wave Type |
|--------------|-----------|------------|----|
| 440.0 | 1.250      | 2.5 | sin |
| 220 | 1.3333333 | 1.66667       | saw |

### Coding language:

The language has these grammar features:

#### Basic note playing
```
play( note, start_time, end_time )
play( chord, start_time, end_time )
```
This will be the most 'basic' command which tells the program to play a single note. An example of how this will be converted to the intermediate language is:

```
play( as4@triangle, 1.25, 3.75 )
```
converts to
| 466.16 | 1.25 | 3.75 | triangle |

Looking at the `note` portion, it follows the following format:  
(note name)(optional sharp/flat)(octave number)@(wave shape)  
note name - Name of the note from a-g. Will only be 1 character  
sharp/flat - This will be implemented using `s` and `b` symbols for sharp and flat respectively. Ofc this is optional  
octave number - a number from 0 to 8  
wave shape - one of the 5 wave shapes as specified above ie. sin, triangle, saw, square, kick  

In addition to this, the `note` can be substituted with a `chord`, which will be explained later.

https://muted.io/note-frequencies/

#### Defining chords

```
chord chordName [array of notes]
```

Using this structure, we can define a 'chord' (which is just a set of notes to play at the same time). This chord then can be used as a shorthand for playing this group of notes. This should only be used in the 'top level' of the code ie. not in any curly braces.

```
chord Cmaj [c3@saw, e3@saw, g3@saw]
play(Cmaj, 2.25, 3)
```

translates to

```
play(c3@saw, 2.25, 3)
play(e3@saw, 2.25, 3)
play(g3@saw, 2.25, 3)
```

In addition to this, you can also define 'compound' chords by using an existing chord name in the list of notes:

```
chord Cmaj7 [Cmaj, b3@saw]
```

translates to 

```
chord Cmaj7 [c3@saw, e3@saw, g3@saw, b3@saw]
```


#### Looping

```
loop(start time, duration, times to loop) {
    stuff to play
  { 
```

This will define a sequence of notes to be looped for several times. 

```
loop(5, 4, 3) {
    play(c3@saw, 0, 1)
    play(e3@saw, 1, 2)
    play(g3@saw, 2, 3)
  }
```

will translate to 

```
// this is the first time playing the loop, offset is 5+0*4 = 5.0
play(c3@saw, 5.0, 6.0)
play(e3@saw, 6.0, 7.0)
play(g3@saw, 7.0, 8.0)

// this is the second time playing the loop, offset is 5+1*4 = 9.0
play(c3@saw, 9.0, 10.0)
play(e3@saw, 10.0, 11.0)
play(g3@saw, 11.0, 12.0)

// this is the third and last time playing the loop, offset is 5+2*4 = 13.0
play(c3@saw, 13.0, 14.0)
play(e3@saw, 14.0, 15.0)
play(g3@saw, 15.0, 16.0)

```

The `loop` portion will get broken down into playing the same thing specified in the curly braces by the specified number of times - `times to loop`. The `start time` will decide the initial offset. 

Also note that the `duration` should limit the notes that you can put in the loop


You could probably pull stupid stuff like playing loops in loops but let's disallow that for now and just let the user put `play` commands in loops


#### Sections

```
section sectionName {
  ...
}

play_section(sectionName, start_time)

```

Something similar to the loops, you can define a `section` which specifies a set of notes to play in the curly braces. Basically when you call `play_section`, it will expand everything in the curly braces, and add the `start_time` value to all the start and end timings of anything in the section.

Unlike the loops, this only has a `start_time` so there are no limits on how long this section will be. What this means is that you can put other (existing) loops and sections in a single section. 

```
section Intro {
...
}
section A {
...
}

section B {
...
}

...

play_section(intro, 0)
play_section(A, 16)
play_section(A, 32)
play_section(B, 48)

```

## LL(1) Grammar Rules

Here are the cfg rules. Terminals are in `code blocks`.

----

START →  Statement

Statement →  

Statement → PlayStatement

Statement → LoopStatement

Statement → ChordStatement

~~Statement → SectionStatement~~ (Do later)

PlayStatement → `play` `(` Note `,` `float` `,` `float` `)`

PlayStatement → `play` `(` `chord name` `,` `float` `,` `float` `)`

Note → `note name` `integer` `@` `wave type` 

LoopStatement → `loop` `(` `float` `,` `float` `,` `integer` `)` `{` Statement `}`



### LL(1) Table

Refer to Pyster's Compiler Basic Chapter 3.12.2: Table-driven LL(1) parsing

We can use this table to parse the LL(1) grammar (or use it as a guideline for recursive descent). If any symbols are not stated in the table, 

----

|Input Symbol| Any |
|--------------|-----------|
|START | Statement|

|Input Symbol| `play` |`loop`|
|--------------|-----------|-|
|Statement| PlayStatement Statement | LoopStatement Statement |

|Input Symbol| `note name` |
|--------------|-----------|
|Note | `note name` `integer` `@` `wave type` |


|Input Symbol| `play` |
|--------------|-----------|
|PlayStatement| `play` `(` Note `,` `float` `,` `float` `)`|

|Input Symbol| `loop` |
|--------------|-----------|
|LoopStatement|`loop` `(` `float` `,` `float` `,` `integer` `)` `{` Statement `}` |



## Translating Into the Intermediate Language

Refer to Pyster's Compiler Basics Chapter 7.5: Generating code from expressions

I'm not really sure if this is how we're supposed to do it

Here is a table of how we can translate the expressions into intermediate code (the csv table)

---

|Trans-Exp||
|--------------|-----------|
|play(note, start, end)| code<sub>1</sub> ++ noteToFrequency(note), start, end\\n|
|loop(start time, duration, times to loop){Statement}| code<sub>1</sub>  ++ Offset(Statement, duration * i for i < times to loop) \\n|


For the offset function, you basically copy and paste the contents of the Statement, but add the offset

|Offset||
|---|---|
|Offset(play(note, start, end))|play(note, start+offset, end+offset)|



TODO
- Find a way to convert a correct CSV to notes for the writer
- Use the CSV to get the maximum buffer size needed ie by taking the maximum ending time
- Let user edit the BPM
- Convert the whole range of notes into their frequencies (https://muted.io/note-frequencies/)




EXTRA FEATURES
- Let the user define chords (groups of notes) to play at one time
- Be able to play different kinds of sound wave shapes (eg. saw wave, triangle wave, square wave)
- Maybe load other wav files eg. drums into the program and write them as notes
- Add looping
