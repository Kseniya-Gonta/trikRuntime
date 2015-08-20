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

#pragma once

#include "i2cInterface.h"

namespace trikHal {
namespace stub {

class StubI2C : public I2CInterface
{
public:
	void send(const QByteArray &data) override;
	int read(const QByteArray &data) override;
	bool connect(const QString &devicePath, int deviceId) override;
	void disconnect() override;
};

}
}
