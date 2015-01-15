/* Copyright 2014 - 2015 CyberTech Labs Ltd.
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

#include <QtCore/QObject>
#include <QtCore/QString>

#include "servoMotor.h"

namespace trikKernel {
class Configurer;
}

namespace trikControl {

/// Angular servomotor.
/// @todo Merge AngularServoMotor and ContiniousRotationServoMotor to one class.
class AngularServoMotor : public ServoMotor
{
	Q_OBJECT

public:
	/// Constructor.
	/// @param port - port on which this motor is configured.
	/// @param configurer - configurer object containing preparsed XML files with motor parameters.
	AngularServoMotor(QString const &port, trikKernel::Configurer const &configurer);

public slots:
	/// Sets current motor angle to specified value.
	/// @param power - servo shaft angle, allowed values are from -90 to 90.
	void setPower(int power);
};

}
