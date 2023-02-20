project(evoBasic)

add_executable(
	ecc
	main.cpp
	${EVO_SHARE_SOURCE_DIR}/analyzer/ast.cpp
	${EVO_SHARE_SOURCE_DIR}/analyzer/context.cpp
	${EVO_SHARE_SOURCE_DIR}/analyzer/helper.cpp
	${EVO_SHARE_SOURCE_DIR}/analyzer/semantic.cpp
	${EVO_SHARE_SOURCE_DIR}/analyzer/symbolCollector.cpp
	${EVO_SHARE_SOURCE_DIR}/analyzer/detailCollector.cpp
	${EVO_SHARE_SOURCE_DIR}/analyzer/type.cpp
	${EVO_SHARE_SOURCE_DIR}/analyzer/typeAnalyzer.cpp
	${EVO_SHARE_SOURCE_DIR}/loader/il.cpp
	${EVO_SHARE_SOURCE_DIR}/loader/loader.cpp
	${EVO_SHARE_SOURCE_DIR}/codegen/ilGen.cpp
	${EVO_SHARE_SOURCE_DIR}/utils/logger.cpp
	${EVO_SHARE_SOURCE_DIR}/utils/cmd.cpp
	${EVO_SHARE_SOURCE_DIR}/utils/i18n.cpp
	${EVO_SHARE_SOURCE_DIR}/parser/token.cpp
	${EVO_SHARE_SOURCE_DIR}/parser/lexer.cpp
	${EVO_SHARE_SOURCE_DIR}/parser/parser.cpp
	${EVO_SHARE_SOURCE_DIR}/parser/parseTree.cpp
	${EVO_SHARE_SOURCE_DIR}/utils/unicode.cpp
	)


