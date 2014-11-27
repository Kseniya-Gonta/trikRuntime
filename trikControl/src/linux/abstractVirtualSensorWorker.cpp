/* Copyright 2014 CyberTech Labs Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. */

#include "src/abstractVirtualSensorWorker.h"

#include <QtCore/QDebug>
#include <QtCore/QFileInfo>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <QsLog.h>

using namespace trikControl;

AbstractVirtualSensorWorker::AbstractVirtualSensorWorker(QString const &script, QString const &inputFile
		, QString const &outputFile)
	: mScript(script)
	, mSensorProcess(this)
	, mInputFile(inputFile)
	, mOutputFile(outputFile)
{
}

AbstractVirtualSensorWorker::~AbstractVirtualSensorWorker()
{
	stop();
}

void AbstractVirtualSensorWorker::stop()
{
	if (mReady) {
		deinitialize();
	}
}

void AbstractVirtualSensorWorker::init()
{
	if (mReady && mInputFile.exists() && mOutputFile.exists()) {
		// Sensor is up and ready.
		return;
	} else if (!mInputFile.exists() || !mOutputFile.exists()) {
		// Sensor is down.
		startVirtualSensor();
	} else {
		// Sensor is already running but we are not connected to it.
		openFifos();
	}
}

void AbstractVirtualSensorWorker::readFile()
{
	char data[4000] = {0};

	mSocketNotifier->setEnabled(false);

	if (::read(mOutputFileDescriptor, data, 4000) < 0) {
		QLOG_ERROR() << mOutputFile.fileName() << ": fifo read failed: " << errno;
		qDebug() << mOutputFile.fileName() << ": fifo read failed: " << errno;
		return;
	}

	mBuffer += data;

	if (mBuffer.contains("\n")) {
		QStringList lines = mBuffer.split('\n', QString::KeepEmptyParts);

		mBuffer = lines.last();
		lines.removeLast();

		for (QString const line : lines) {
			onNewData(line);
		}
	}

	mSocketNotifier->setEnabled(true);
}

bool AbstractVirtualSensorWorker::launchSensorScript(QString const &command)
{
	QLOG_INFO() << "Sending" << command << "command to" << sensorName() << "sensor";
	qDebug() << "Sending" << command << "command to" << sensorName() << "sensor";

	QFileInfo const scriptFileInfo(mScript);

	if (mSensorProcess.state() == QProcess::Running) {
		mSensorProcess.close();
	}

	QStringList const params{command};

	mSensorProcess.setWorkingDirectory(scriptFileInfo.absolutePath());
	mSensorProcess.start(scriptFileInfo.filePath(), params, QIODevice::ReadOnly | QIODevice::Unbuffered);

	mSensorProcess.waitForStarted();

	if (mSensorProcess.state() != QProcess::Running) {
		QLOG_ERROR() << "Cannot launch" << sensorName() << "script" << scriptFileInfo.filePath() << " in "
				<< scriptFileInfo.absolutePath();
		qDebug() << "Cannot launch" << sensorName() << "script" << scriptFileInfo.filePath() << " in "
				<< scriptFileInfo.absolutePath();
		return false;
	}

	if (!mSensorProcess.waitForFinished()) {
		QLOG_ERROR() << sensorName() << "script" << scriptFileInfo.filePath() << " in " << scriptFileInfo.absolutePath()
				<< "hanged up or finished unexpectedly!";
		qDebug() << sensorName() << "script" << scriptFileInfo.filePath() << " in " << scriptFileInfo.absolutePath()
				<< "hanged up or finished unexpectedly!";
		return false;
	}

	QString const processOutput = mSensorProcess.readAllStandardOutput() + mSensorProcess.readAllStandardError();
	if (processOutput.contains("error")) {
		QLOG_ERROR() << sensorName() << "script reported error:" << processOutput;
		qDebug() << sensorName() << "script reported error:" << processOutput;
		return false;
	}

	QLOG_INFO() << "Sensor process output:" << processOutput;
	qDebug() << "Sensor process output:" << processOutput;

	return true;
}

void AbstractVirtualSensorWorker::startVirtualSensor()
{
	if (launchSensorScript("start")) {
		QLOG_INFO() << sensorName() << "sensor started, waiting for it to initialize...";
		qDebug() << sensorName() << "sensor started, waiting for it to initialize...";
		openFifos();
	}
}

void AbstractVirtualSensorWorker::openFifos()
{
	if (mInputFile.isOpen()) {
		mInputFile.close();
	}

	QLOG_INFO() << "Opening" << mOutputFile.fileName();
	qDebug() << "Opening" << mOutputFile.fileName();

	/// @todo Is it possible to use QFile (and QFile::handle()) here?
	mOutputFileDescriptor = open(mOutputFile.fileName().toStdString().c_str(), O_SYNC | O_NONBLOCK, O_RDONLY);

	if (mOutputFileDescriptor == -1) {
		QLOG_ERROR() << "Cannot open" << sensorName() << "sensor output file" << mOutputFile.fileName();
		qDebug() << "Cannot open" << sensorName() << "sensor output file" << mOutputFile.fileName();
		return;
	}

	mSocketNotifier.reset(new QSocketNotifier(mOutputFileDescriptor, QSocketNotifier::Read));

	connect(mSocketNotifier.data(), SIGNAL(activated(int)), this, SLOT(readFile()));
	mSocketNotifier->setEnabled(true);

	QLOG_INFO() << "Opening" << mInputFile.fileName();
	qDebug() << "Opening" << mInputFile.fileName();

	if (!mInputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QLOG_ERROR() << "Sensor input file" << mInputFile.fileName() << " failed to open";
		qDebug() << "Sensor input file" << mInputFile.fileName() << " failed to open";
		return;
	}

	mInputStream.setDevice(&mInputFile);

	mReady = true;

	QString const message = sensorName() + " initialization completed";

	QLOG_INFO() << message;
	qDebug() << message;

	sync();
}

void AbstractVirtualSensorWorker::sendCommand(QString const &command)
{
	mCommandQueue << command;
	sync();
}

void AbstractVirtualSensorWorker::deinitialize()
{
	if (mSocketNotifier) {
		disconnect(mSocketNotifier.data(), SIGNAL(activated(int)), this, SLOT(readFile()));
		mSocketNotifier->setEnabled(false);
	}

	if (::close(mOutputFileDescriptor) != 0) {
		QString const message = QString("%1: fifo close failed: %2").arg(mOutputFile.fileName()).arg(errno);
		QLOG_ERROR() << message;
		qDebug() << message;
	}

	mOutputFileDescriptor = -1;
	mInputFile.close();

	if (!launchSensorScript("stop")) {
		QString const message = QString("Failed to stop %1 sensor!").arg(sensorName());
		QLOG_ERROR() << message;
		qDebug() << message;
	} else {
		QString const message = QString("Successfully stopped %1 sensor").arg(sensorName());
		QLOG_INFO() << message;
		qDebug() << message;
	}

	emit stopped();

	mReady = false;
}

void AbstractVirtualSensorWorker::sync()
{
	if (mReady) {
		for (QString const &command : mCommandQueue) {
			mInputStream << command + "\n";
			mInputStream.flush();
		}

		mCommandQueue.clear();
	}
}
