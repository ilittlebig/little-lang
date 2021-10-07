" Vim syntax file
" Language:		   Little
" Maintainer:	   N/A
" Latest Revision: 25 September 2021

" Add this file to
"	~/.vim/syntax
" Add this command to your .vimrc file
"	autocmd BufRead,BufNewFile *.lil setfiletype lil

if exists("b:current_syntax")
	finish
endif

" Keywords
syn keyword littleLangKeywords return if else fn int void string defvar float

" Comments
syn match littleComment oneline "//.*$"
syn region littleCommentStart start=+/\*+ end=+\*/+  extend fold

" Strings
syn region littleString start='"' end='"'

" Numbers
syn match littleNumber /\d\+\(u\=l\{0,2}\|ll\=u\)\>/

" Highlight
let b:current_syntax = "lil"

hi def link littleLangKeywords Keyword
hi def link littleComment	   Comment
hi def link littleCommentStart Comment
hi def link littleString	   String
hi def link littleNumber       Number
