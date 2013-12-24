if exists("b:current_syntax")
    finish
endif

let b:current_syntax = "wl"

syn keyword External import module
syn keyword ScopeDecl package
syn keyword Assert assert
syn keyword Conditional if else switch
syn keyword Branch goto break continue
syn keyword Repeat while for do foreach
syn keyword Bool true false
syn keyword Constant null
syn keyword Typedef alias
syn keyword Statement return
hi def link ScopeDecl StorageClass

" Types
syn keyword Structure class struct union
syn keyword Enum enum
syn keyword Type char uchar byte ubyte short ushort int uint long ulong
syn keyword Type int8 uint8 int16 uint16 int32 uint32 int64 uint64
syn keyword Type void bool
syn keyword Type float double
syn keyword Type float32 float64

" Comments
syn match wlTodo contained "\<\(TODO\|FIXME\|XXX\|NOTE\|TEMP\|HACK\|BUG\|REVIEW\|REFACTOR\)\(:\)\="
syn region BlockComment start="/\*" end="\*/" contains=wlTodo,@Spell fold
syn match LineComment "//.*" contains=wlTodo,@Spell
hi def link wlTodo Todo
hi def link LineComment Comment
hi def link BlockComment Comment

" Numbers
syn match Dec display "\<\d[0-9]*\>"
hi def link Dec Number

" Strings
syn region String start=+"+ end=+"[cwd]\=+ contains=@Spell
