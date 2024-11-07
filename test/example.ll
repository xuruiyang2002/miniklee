; ModuleID = './test/example.c'
source_filename = "./test/example.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 !dbg !10 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  call void @llvm.dbg.declare(metadata i32* %2, metadata !16, metadata !DIExpression()), !dbg !17
  store i32 1, i32* %2, align 4, !dbg !17
  call void @llvm.dbg.declare(metadata i32* %3, metadata !18, metadata !DIExpression()), !dbg !19
  store i32 2, i32* %3, align 4, !dbg !19
  call void @llvm.dbg.declare(metadata i32* %4, metadata !20, metadata !DIExpression()), !dbg !21
  %6 = load i32, i32* %2, align 4, !dbg !22
  %7 = load i32, i32* %3, align 4, !dbg !23
  %8 = add nsw i32 %6, %7, !dbg !24
  store i32 %8, i32* %4, align 4, !dbg !21
  call void @llvm.dbg.declare(metadata i32* %5, metadata !25, metadata !DIExpression()), !dbg !27
  store i32 0, i32* %5, align 4, !dbg !27
  br label %9, !dbg !28

9:                                                ; preds = %15, %0
  %10 = load i32, i32* %5, align 4, !dbg !29
  %11 = icmp slt i32 %10, 5, !dbg !31
  br i1 %11, label %12, label %18, !dbg !32

12:                                               ; preds = %9
  %13 = load i32, i32* %2, align 4, !dbg !33
  %14 = add nsw i32 %13, 2, !dbg !35
  store i32 %14, i32* %2, align 4, !dbg !36
  br label %15, !dbg !37

15:                                               ; preds = %12
  %16 = load i32, i32* %5, align 4, !dbg !38
  %17 = add nsw i32 %16, 1, !dbg !38
  store i32 %17, i32* %5, align 4, !dbg !38
  br label %9, !dbg !39, !llvm.loop !40

18:                                               ; preds = %9
  ret i32 0, !dbg !43
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nofree nosync nounwind readnone speculatable willreturn }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7, !8}
!llvm.ident = !{!9}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "Ubuntu clang version 14.0.0-1ubuntu1.1", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test/example.c", directory: "/home/xry/workspace/tmp/toy-symexe/llvm-framework", checksumkind: CSK_MD5, checksum: "691fef11c07f76026e51a5cce6c085b1")
!2 = !{i32 7, !"Dwarf Version", i32 5}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 7, !"PIC Level", i32 2}
!6 = !{i32 7, !"PIE Level", i32 2}
!7 = !{i32 7, !"uwtable", i32 1}
!8 = !{i32 7, !"frame-pointer", i32 2}
!9 = !{!"Ubuntu clang version 14.0.0-1ubuntu1.1"}
!10 = distinct !DISubprogram(name: "main", scope: !11, file: !11, line: 1, type: !12, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !15)
!11 = !DIFile(filename: "./test/example.c", directory: "/home/xry/workspace/tmp/toy-symexe/llvm-framework", checksumkind: CSK_MD5, checksum: "691fef11c07f76026e51a5cce6c085b1")
!12 = !DISubroutineType(types: !13)
!13 = !{!14}
!14 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!15 = !{}
!16 = !DILocalVariable(name: "a", scope: !10, file: !11, line: 2, type: !14)
!17 = !DILocation(line: 2, column: 9, scope: !10)
!18 = !DILocalVariable(name: "b", scope: !10, file: !11, line: 3, type: !14)
!19 = !DILocation(line: 3, column: 9, scope: !10)
!20 = !DILocalVariable(name: "c", scope: !10, file: !11, line: 4, type: !14)
!21 = !DILocation(line: 4, column: 9, scope: !10)
!22 = !DILocation(line: 4, column: 13, scope: !10)
!23 = !DILocation(line: 4, column: 17, scope: !10)
!24 = !DILocation(line: 4, column: 15, scope: !10)
!25 = !DILocalVariable(name: "i", scope: !26, file: !11, line: 10, type: !14)
!26 = distinct !DILexicalBlock(scope: !10, file: !11, line: 10, column: 5)
!27 = !DILocation(line: 10, column: 14, scope: !26)
!28 = !DILocation(line: 10, column: 10, scope: !26)
!29 = !DILocation(line: 10, column: 21, scope: !30)
!30 = distinct !DILexicalBlock(scope: !26, file: !11, line: 10, column: 5)
!31 = !DILocation(line: 10, column: 23, scope: !30)
!32 = !DILocation(line: 10, column: 5, scope: !26)
!33 = !DILocation(line: 11, column: 13, scope: !34)
!34 = distinct !DILexicalBlock(scope: !30, file: !11, line: 10, column: 33)
!35 = !DILocation(line: 11, column: 15, scope: !34)
!36 = !DILocation(line: 11, column: 11, scope: !34)
!37 = !DILocation(line: 12, column: 5, scope: !34)
!38 = !DILocation(line: 10, column: 29, scope: !30)
!39 = !DILocation(line: 10, column: 5, scope: !30)
!40 = distinct !{!40, !32, !41, !42}
!41 = !DILocation(line: 12, column: 5, scope: !26)
!42 = !{!"llvm.loop.mustprogress"}
!43 = !DILocation(line: 13, column: 5, scope: !10)
