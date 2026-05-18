/**
 * @file geninterp.hpp
 * @author Ondřej Turek, xtureko00
 * @brief Code generator interface
 */
#ifndef _GENINTERP_H
#define _GENINTERP_H

#include "editorstate.hpp"
#include "variableeditor.hpp"
#include <filesystem>
#include <qchar.h>
#include <qobject.h>
#include <qobjectdefs.h>
#include <qstringliteral.h>
#include <sstream>
#include <string>
#include <QString>
#include <QProcess>

namespace {
    const std::string MAIN_FILENAME = "program.cpp";
    const std::string MAIN_GENERATED = "program.generated.cpp";
    const std::string GENERATED_EXE = "generated.out";
}

class InterpreterGenerator : public QObject {
    Q_OBJECT
    public:
        explicit InterpreterGenerator(QObject *parent = nullptr) {};

        /// @brief Sets the path to the interpreter source code
        void setPath(const std::filesystem::path path);

        /// @brief Sets the file name of the entry point source file
        void setEntrypoint(const std::string filename);

        /// @brief Sets the marker to replace with the net definition
        void setMarker(const std::string);

        /// @brief Generates an entry point source file for the interpreter from the specification
        bool generateMain(const PetriNetworkSpec *spec);

        /// @brief Compiles the interpreter from the source and generated entry point file
        void compile();

        /// @brief Runs the interpreter
        void run();

        /// @brief Terminates the interpreter process
        void kill();

    signals:
        /// @brief Emitted when the compiler succesfully starts
        void compileStarted();

        /// @brief Emitted when the compiler writes to the standard output
        void compileProgress(QString stdout);

        /// @brief Emitted when the compiler exits with an error
        void compileFailed();

        /// @brief Emitted when the compiler exits succesfully
        void compileFinished();

        /// @brief Emitted when the interpreter starts
        void interpreterStarted();

        /// @brief Emitted when the interpreter writes to stdout
        void interpreterOutput(QString line);

        /// @brief Emitted when the interpreter exits
        void interpreterStopped(int exitCode);

        /// @brief Emitted when the compiler exits with an error
        void interpreterError(QString reason);
    
    private:
        /// @brief Inserts the definition of a place into the buffer
        void emitPlace(const PetriPlace *p);

        /// @brief Inserts the definition of a transition into the buffer
        void emitTransition(const PetriTransition *t);

        /// @brief Inserts the definition of an arc into the buffer
        void emitArc(const PetriArc *a);

        void emitVariables(const VariableMap *vars);

        /// @brief Emits all arcs, transitions and places from the passed specification
        void emitAll(const PetriNetworkSpec *spec);

        /// @brief A buffer containing the code to be inserted into the program's code
        std::ostringstream generatedBuffer;

        /// @brief Path to the directory containing the interpreter's source code
        std::filesystem::path interpSourcePath;

        /// @brief Content of the comment marking the definition point in the source code
        QString interpDefMarker;
        
        QProcess *compileProcess = nullptr; ///< Handle to the compiler process
        QProcess *interpreterProcess = nullptr; ///< Handle to the interpreter process

    private slots:
        void onReadyReadStdout();
        void onReadyReadStderr();
        void onProcessFinished(int exitCode, QProcess::ExitStatus status);
        void onProcessError(QProcess::ProcessError error);
};

#endif



    