/* Copyright 2015 Yurii Litvinov and CyberTech Labs Ltd.
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

#include "stubHardwareAbstraction.h"

#include "stubI2c.h"
#include "stubSystemConsole.h"
#include "stubEventFile.h"
#include "stubInputDeviceFile.h"
#include "stubOutputDeviceFile.h"
#include "stubFifo.h"

using namespace trikHal;
using namespace trikHal::stub;

StubHardwareAbstraction::StubHardwareAbstraction()
	: mI2c(new StubI2C())
	, mSystemConsole(new StubSystemConsole())
{
}

StubHardwareAbstraction::~StubHardwareAbstraction()
{
}

I2CInterface &StubHardwareAbstraction::i2c()
{
	return *mI2c.data();
}

SystemConsoleInterface &StubHardwareAbstraction::systemConsole()
{
	return *mSystemConsole.data();
}

EventFileInterface *StubHardwareAbstraction::createEventFile() const
{
	return new StubEventFile();
}

FifoInterface *StubHardwareAbstraction::createFifo() const
{
	return new StubFifo();
}

InputDeviceFileInterface *StubHardwareAbstraction::createInputDeviceFile(const QString &fileName) const
{
	return new StubInputDeviceFile(fileName);
}

OutputDeviceFileInterface *StubHardwareAbstraction::createOutputDeviceFile(const QString &fileName) const
{
	return new StubOutputDeviceFile(fileName);
}
