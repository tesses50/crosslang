## CrossLang Handbook

This is a book for my language found [here](https://crosslang.tesseslanguage.com/)



## Statements and loops



#### Declare a variable (it is an expression)

You can (the var is optional but should be used if the root environment has the same variable and you don't want to set it)

```js
var myVariable = 42;
```

#### Set a variable (it is an expression)

```js
myVariable = "I have set the variable to a string";
```

#### If statements

The {} are optional if you only need one statement

```js
var myExpression = 0; 
var myOtherExpression = {
    mol = 42
};

if(myExpression)
{
    Console.WriteLine("myExpression is truthy");
}
else if(myOtherExpression)
{
    //this will run based on the condition
    Console.WriteLine("myExpression is falsey but myOtherExpression is truthy");
}
else {
     Console.WriteLine("both myExpression and myOtherExpression are falsey");
}
```

#### For loop

Note: i will leak, also you can use curly brackets like if

```js
for(var i = 0; i < 42; i++)
    Console.WriteLine(i);
```

#### Each loop

```js
enumerable MyItterator()
{
    yield 42;
    yield 100;
    yield 88;
}
each(var item : MyItterator())
{
    Console.WriteLine(item);
}


each(var item : [3,7,12])
{
    Console.WriteLine(item);
}


/*
    This should output
    42
    100
    88
    3
    7
    12
*/
```

#### While loop

```js
while(conditionLikeIf)
{
    Console.WriteLine("Hi");
}

//If conditionLikeIf is falsey we never print Hi
//If conditionLikeIf is truthy we print Hi until it is falsey
```

#### Do loop

```js
do(conditionLikeIf)
{
    Console.WriteLine("Hi");
}

//If conditionLikeIf is falsey we print Hi once
//If conditionLikeIf is truthy we print Hi until it is falsey
```



#### Try statement

Note: you can't yield, return, break or continue from inside a try statement (also applys to catch and finally due to these being internally closures)

```js
try {
    Console.WriteLine("God is good.");
    throw "my error";
    Console.WriteLine("I am lonely, I am never called.");
} 
catch(ex)
{
     Console.WriteLine($"I got an error {ex}.");
} 
finally {
     Console.WriteLine("For God so loved the world that he gave his only begotten Son, that whoever believes in him should not perish but have eternal life.");  
}


/*
    This should print
    God is good.
    I got an error my error.
    For God so loved the world that he gave his only begotten Son, that whoever believes in him should not perish but have eternal life.
*/
```

#### Switch statement

NOTE: you can use dictionaries with overloaded equals operator (we will pass the switch's expression as the argument)

```js
o = {
    operator== = (this,v) => {
          //this is implicitly passed by runtime, but you must declare it if you want it
          return (v * 7) == 42;
    }
};

var val = 6;

switch(val)
{
    case 4:
        Console.WriteLine("I am four");
        break;
    case 5:
        Console.WriteLine("I am five");
        break;
    case o:
        Console.WriteLine("Hitchhiker's guide to the galaxy meaning of life");
        break;
    default:
        Console.WriteLine("We don't support it we are sorry");
}
```



#### Function declaration

```go
func My.Func.Is.A.Few.Deep(a, b)
{
    return a * b;
}
func main(args)
{
    Console.WriteLine(My.Func.Is.A.Few.Deep(6,7));
}
```

#### Function inside a function

Oh yeah functions can have exclusively a expression as a body which does not need a return keyword

```go
func main(args)
{
    func MyOther(a,b) a * b;
    Console.WriteLine(MyOther(a,b));
}
```

#### Document a function

```go
/^ 
    This function returns 42
^/
func MyFunction() 42;
```

#### Return statement

You can also return nothing

```js
return 42;
return;
```

#### Yield statement

This will halt the function's execution and return the running function object (were you can pop the item and resume it)

To automaticly make a function enumerable (with each statement) you must use ``enumerable`` instead of func when declaring your function

```js
yield 42;
```

#### Throw statement

You can catch this with ``try statement`` with ``catch`` clause

```js
throw "You can replace me with another string or any other object";
```

#### Defer statement

Your code will be called when your scope is destroyed

```go
{
    defer {
        Console.WriteLine("In defer");
    }
    Console.WriteLine("About to leave scope");
}
Console.WriteLine("Left scope");


/*
The output should be

About to leave scope
In defer
Left scope
*/
```

## Expressions

We will place it as a parameter to a function ``doThing`` but these will work anywhere like setting a variable or another expression

#### Undefined

```js
doThing(undefined);
```

#### Null

```js
doThing(null);
```

#### Boolean

```js
doThing(false);
doThing(true);
```

#### Long

A signed 64 bit number

```js
doThing(94);
doThing(599929022);
```

#### Double

A 64 bit floating pointer number

```js
doThing(5.25);
doThing(942.90204);
```

#### Char

Yes we support single character strings (this escapes a double quote)

```csharp
doThing('\"');
```

#### String

Second is an interopolated string

```csharp
doThing("Hi");
doThing($"My name is {name}");
```

#### List

```js
doThing(
    [4,5,9,42,true,"Hi",'\n']
);
```

#### Dictionary

```js
doThing(
    {
        a = "This is an option",
        b = 42,
        c = 9
    }
);
```

###### Enumerating a dictionary

```js
var dict = { a = 5, b = 42};
each(var item : Dictionary.Items(dict))
{
    Console.WriteLine($"Key: {item.Key}");
    Console.WriteLine($"Value: {item.Value}");
    Console.WriteLine();
}
/*
This prints
Key: a
Value: 5

Key: b
Value: 42

*/
```



###### Getting a value from dictionary

```go
func doThing(data)
{
   Console.WriteLine(data.b); //will print 42
}
```

###### Setting a value to dictionary

```go
func doThing(data)
{
    data.a = true; //replace a with a different value
    data.someFieldThatDoesNotExist = 9; //define a new field entirely

}
```

###### Methods

Note that this is optional but if you do have it it must be the first argument, the this argument points to the dictionary

```go
var item = {
      myMethod = (this, a, b) => {
            return a * b;
      }
};

Console.WriteLine(item.myMethod(6,7)); //prints 42
```

Or

```go
var item = {};
func item.myMethod(this, a, b)
{
    return  a * b;
}
Console.WriteLine(item.myMethod(6,7)); //prints 42
```

###### Properties

They are declared like every other method except they are prefixed with ``get`` or ``set``

```go
var item = {
      getProp = (this)=>{
          return 42;  
      },
      setProp = (this,v)=>{
           //do something with v
      }
};

Console.WriteLine(item.Prop); //equivalent to Console.WriteLine(item.getProp());

item.Prop = 100; //equivalent to item.setProp(100); will do nothing in this example

```

###### Operator overloading

```js
var item = {
        operator+ = (this,right) => {
             return 4 + a;
        }
        operator- = (this,right) => {
            return 4 - right; //be careful right may be undefined if the negate operator is used
        }
};

Console.WriteLine(item + 5); //prints 9
```

###### Array operator overloading

```js
var item = {
    GetAt = (index)=>{
        return index * 2; //we multiply index by two and return
    },
    SetAt => (index,value)=>{
        Console.WriteLine($"The index was {index} and value was {value}");
    }
};

Console.WriteLine(item[21]); //prints 42
item["Hi"] = "John"; //prints The index was Hi and value was John
```

#### ByteArray

```js
var bA = ByteArray(1);
ba[0] = 42;
Console.WriteLine(ba.ToString()); //prints *
```

###### ByteArray from string

```js
var bA = ByteArray("Hi");
//we can get bytes like this bA[0] which will be 72
//we can get length by bA.Count or bA.Length
```

#### Embedding files (you can't do this easily in C/C++ and C# not as easy as this)

```js
var bA = embed("myFile.png"); //will embed the file res/myFile.png
//it is stored as a RESO in the crvm file and bA is a ByteArray
//note the argument MUST be a constant string
//note you can access a subdirectory of res as well just use dir/file.bin
```

#### Streams



###### From an existing file

```js
var strm = FS.Local.OpenFile("file.bin","rb");

var data = ByteArray(1024);
var read = strm.Read(data,0,data.Count); 
//use ReadBlock to ensure you have 1024 bytes if not near end of stream

//use WriteBlock to ensure you write the amount you say you need

strm.Close();
```

#### VFS

```js
var myVFS = FS.Local; //FS.Local is the local filesystem


myVFS.CreateDirectory("myDir"); //creates directory in current dir (only local)

myVFS.DeleteDirectory("myDir"); //now we delete it


myVFS.CreateDirectory(Path.FromString(Env.Downloads) / "Folder in Downloads"); //Create a folder in downloads
```



#### Path

```js
var path = / "Path" / "To" / "Directory";
var relativePath = ./"another"/"path";
var combined = path / relativePath;
var someString = "joel";
var combinedAndStr = combined / someString;
var combinedWithExtension = combined + ".mp4";
Console.WriteLine(combined); // outputs /Path/To/Directory/another/path
Console.WriteLine(combined); // outputs /Path/To/Directory/another/path/joel
Console.WriteLine(combinedWithExtension); //outputs /Path/To/Directory/another/path.mp4
//these paths are used with vfs


//to get parent directory use combined.GetParent();
//to get filename use combined.GetFileName();
//to get extension use combinedWithExtension.GetExtension();, it will be the extension including .
//to change the extension use combinedWithExtension.ChangeExtension(".mkv");
//to remove the extension use combinedWithExtension.ChangeExtension(null); or combinedWithExtension.ChangeExtension();
```




























