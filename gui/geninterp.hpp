/**
 * @file geninterp.hpp
 * @author Ondřej Turek, xtureko00
 * @brief Rozhraní generátoru kódu interpretu
 */
#ifndef _GENINTERP_H
#define _GENINTERP_H

#include "editorstate.hpp"
#include <filesystem>
#include <qchar.h>
#include <qobject.h>
#include <qobjectdefs.h>
#include <sstream>
#include <string>
#include <QString>
#include <QProcess>

const std::string MAIN_FILENAME = "program.cpp";
const std::string MAIN_GENERATED = "program.generated.cpp";

class InterpreterGenerator : public QObject {
    Q_OBJECT
    public:
        explicit InterpreterGenerator(QObject *parent = nullptr) {};
        void setPath(const std::filesystem::path path);
        std::filesystem::path getPath();
        void setEntrypoint(const std::string filename);
        void setMarker(const std::string);
        bool generateMain(const PetriNetworkSpec *spec);
        void compile();
        bool run();

    signals:
        void compileStarted();
        void compileProgress(QString stdout);
        void compileFailed();
        void compileFinished();
    
    private:
        void emitPlace(const PetriPlace *p);
        void emitTransition(const PetriTransition *t);
        void emitArc(const PetriArc *a);
        void emitAll(const PetriNetworkSpec *spec);

        std::filesystem::path interpSourcePath;
        std::string interpEntryFilename;
        QString interpDefMarker;
        std::ostringstream generatedBuffer;
        QProcess *compileProcess;
        QString stderrOutput;

    private slots:
        void onReadyReadStdout();
        void onReadyReadStderr();
        void onProcessFinished(int exitCode, QProcess::ExitStatus status);
        void onProcessError(QProcess::ProcessError error);
};

#endif