project(evoBasic)

add_executable(
		evm
		main.cpp
        ../share/execution/runtime.cpp
		processor.cpp
		${EVO_SHARE_SOURCE_DIR}/utils/logger.cpp
		${EVO_SHARE_SOURCE_DIR}/utils/cmd.cpp
		${EVO_SHARE_SOURCE_DIR}/loader/il.cpp
		${EVO_SHARE_SOURCE_DIR}/loader/loader.cpp
		${EVO_SHARE_SOURCE_DIR}/analyzer/type.cpp
		${EVO_SHARE_SOURCE_DIR}/analyzer/ast.cpp
		${EVO_SHARE_SOURCE_DIR}/analyzer/context.cpp
		${EVO_SHARE_SOURCE_DIR}/parser/token.cpp
		${EVO_SHARE_SOURCE_DIR}/loader/bytecode.cpp
		${EVO_SHARE_SOURCE_DIR}/execution/intrinsic.cpp
)


