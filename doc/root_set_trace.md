# GC根集追踪

- √ 表示该操作需要追踪
- x 表示该操作不需要追踪
- 留空表示该操作不存在

|操作        	|向操作栈写入引用|擦除操作栈中的引用|向变量写入引用|擦除变量中的引用|
|-|-|-|-|-|
|starg.ref	| |√|√|√|
|ldarg.ref	|√| | | |
|stloc.ref	| |√|√|√|
|ldloc.ref	|√| | | |
|stfld.ref	| |√|x|x|
|ldfld.ref	|√| | | |
|stsfld.ref	| |√|√|√|
|ldsfld.ref	|√| | | |
|stelemr.ref	| |√|x|x|
|stelem.ref	| |√|x|x|
|ldelem.ref	|√| | | |
|newarray	|√| | | |
|dup.ref	|√| | | |
|pop.ref	| |√| | |
|store.ref	| |√|?|?|
|load.ref	|√| | | |
|instanceof	| |√| | |
|ldstr		|√| | | |
|newobj		|√| | | |
|throw		| |√| | |
|压入函数栈       | |√|√| |
|弹出函数栈	| | | |√|





