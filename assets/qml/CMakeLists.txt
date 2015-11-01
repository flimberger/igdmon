set(QML_FILES appui.qml)

add_custom_target(lint)
foreach(QML_FILE ${QML_FILES})
    add_custom_command(TARGET lint COMMAND qmllint ${QML_FILE} COMMENT "Linting ${QML_FILE}")
endforeach()
