Imports System
Imports Tinkerforge

' FIXME: This example is incomplete

Module ExampleCallback
    Const HOST As String = "localhost"
    Const PORT As Integer = 4223
    Const UID As String = "XYZ" ' Change XYZ to the UID of your Thermal Imaging Bricklet

    ' Callback subroutine for high contrast image callback
    Sub HighContrastImageCB(ByVal sender As BrickletThermalImaging, _
                            ByVal image As Byte())

    End Sub

    Sub Main()
        Dim ipcon As New IPConnection() ' Create IP connection
        Dim ti As New BrickletThermalImaging(UID, ipcon) ' Create device object

        ipcon.Connect(HOST, PORT) ' Connect to brickd
        ' Don't use device before ipcon is connected

        ' Register high contrast image callback to subroutine HighContrastImageCB
        AddHandler ti.HighContrastImageCallback, AddressOf HighContrastImageCB

        ' Enable high contrast image transfer for callback
        ti.SetImageTransferConfig(BrickletThermalImaging.IMAGE_TRANSFER_CALLBACK_HIGH_CONTRAST_IMAGE)

        Console.WriteLine("Press key to exit")
        Console.ReadLine()
        ipcon.Disconnect()
    End Sub
End Module
