if exists("b:current_syntax")
    finish
endif

set smartindent
set indentexpr="0{,0}"

let b:current_syntax = "wl"

syn match wlImportId "C" contained
syn match wlImport "\<import\(\s*([_a-zA-Z][_a-zA-Z0-9]*)\)\?" contains=wlImportId
hi def link wlImport Include
hi def link wlImportId Identifier

syn keyword wExternal module
syn keyword wScopeDecl package
syn keyword wAssert assert
syn keyword wConditional if else switch
syn keyword wBranch goto break continue
syn keyword wRepeat while for do foreach
syn keyword wBool true false
syn keyword wConstant null
syn keyword wTypedef alias
syn keyword wLabel case default label
syn keyword wStatement return
syn keyword wStorageClass extern undecorated implicit weak const
syn keyword wOperator and or not
syn keyword wMemOperator new delete renew retain release
syn keyword wDeclaration var this
syn keyword wUse use
" reserved keywords
syn keyword wReserve explicit decorated defer foreach asm once number let super

hi def link wExternal Include
hi def link wScopeDecl StorageClass
hi def link wAssert Assert
hi def link wConditional Conditional
hi def link wBranch Statement
hi def link wRepeat Repeat
hi def link wBool Boolean
hi def link wConstant Constant
hi def link wTypedef Typedef
hi def link wLabel Label
hi def link wStatement Statement
hi def link wStorageClass StorageClass
hi def link wOperator Operator
hi def link wMemOperator Operator
hi def link wDeclaration Keyword
hi def link wUse Keyword
hi def link wReserve Error

" Types
syn keyword Structure struct union function interface class
syn keyword Enum enum
syn keyword Type char uchar byte ubyte short ushort int uint long ulong
syn keyword Type int8 uint8 int16 uint16 int32 uint32 int64 uint64
syn keyword Type void bool
syn keyword Type float double
syn keyword Type float32 float64

" Comments
syn match wTodo contained "\<\(TODO\|FIXME\|XXX\|NOTE\|TEMP\|HACK\|BUG\|REVIEW\|REFACTOR\)\(:\)\="
syn region wBlockComment start="/\*" end="\*/" contains=wTodo,@Spell fold
syn match wLineComment "//.*" contains=wTodo,@Spell
hi def link wTodo Todo
hi def link wLineComment Comment
hi def link wBlockComment Comment

" Numbers
syn case ignore
syn match wDec display "\<\d[0-9_]*\>"
syn match wHex display "\<0x[0-9a-f_]\+\>"
syn match wBin display "\<0b[01_]\+\>"
syn match wOct display "\<0o[0-7_]\+\>"
syn match wFloat display "\<\d[0-9_]*f\>"
syn match wFloat display "\<\d[0-9_]*\.[0-9_]*\(e[-+]\=[0-9_]\+\)\=[f]\="
syn case match
hi def link wDec Number
hi def link wHex Number
hi def link wBin Number
hi def link wOct Number
hi def link wFloat Float

" Strings
syn region String start=+"+ end=+"[cwd]\=+ contains=@Spell

" braces
syn region wBlock start="{" end="}" transparent fold
syn region wParen start="(" end=")" transparent
