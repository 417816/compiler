# compiler
自作コンパイラ

## Overview
このコンパイラは，再帰下降型の構文解析と構文主導型のコード生成に基づく1パスのコンパイラである．

## Description


### Syntax
プログラム → OPT[ var 変数宣言 ] SEQ[ fun 関数定義 ] END_OF_INPUT

変数宣言   → 識別子 , … ;

関数定義   → void 識別子 ( OPT[ 識別子 , … ] ) { OPT[ var 変数宣言 ] SEQ 文 }
           ｜ int  識別子 ( OPT[ 識別子 , … ] ) { OPT[ var 変数宣言 ] SEQ 文 }

文         → 識別子 ( OPT[ 式 , … ］) ;
           ｜ 識別子 = 式 ;
           ｜ if ( 式 ) 文 OPT[ else 文 ]
           ｜ while ( 式 ) 文
           ｜ { SEQ 文 }
           ｜ return OPT 式 ;

式         → 比較式 [ == ｜ != ] …

比較式     → 算術式 [ > ｜ >= ｜ < ｜ <= ] …

算術式     → 項 [ + ｜ - ] …

項         → 因子 [ * ｜ / ｜ % ] …

因子       → OPT[ + ｜ - ] 原子式

原子式     → 数
           ｜ ( 式 )
           ｜ 識別子
           ｜ 識別子 ( OPT[ 式 , … ] )

## Usage
make  
./compile 原始プログラム > 目的コード  
./run 目的コード
### example
make  
./compile scope.cpr > scope.pco  
./run scope.pco
