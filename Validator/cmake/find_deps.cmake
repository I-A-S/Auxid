include(FetchContent)

FetchContent_Declare(
    Fixpoint
    GIT_REPOSITORY https://github.com/I-A-S/Fixpoint
    GIT_TAG        main
    OVERRIDE_FIND_PACKAGE
)

FetchContent_MakeAvailable(Fixpoint)