Remove-Item -Recurse -Force .\x64\Release\** -ErrorAction SilentlyContinue
Remove-Item -Recurse -Force .\x86\Release\** -ErrorAction SilentlyContinue
Remove-Item -Recurse -Force .\TqkLibrary.WindowCapture\bin\Release\** -ErrorAction SilentlyContinue

$env:PATH="$($env:PATH);C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE"
devenv .\TqkLibrary.WindowCapture.sln /Rebuild 'Release|x64' /Project TqkLibrary.WindowCapture.Native
devenv .\TqkLibrary.WindowCapture.sln /Rebuild 'Release|x86' /Project TqkLibrary.WindowCapture.Native

dotnet build --no-incremental .\TqkLibrary.WindowCapture\TqkLibrary.WindowCapture.csproj -c Release
nuget pack .\TqkLibrary.WindowCapture\TqkLibrary.WindowCapture.nuspec -Symbols -OutputDirectory .\TqkLibrary.WindowCapture\bin\Release

$localNuget = $env:localNuget
if(![string]::IsNullOrWhiteSpace($localNuget))
{
    Copy-Item .\TqkLibrary.WindowCapture\bin\Release\*.nupkg -Destination $localNuget -Force
}

$nugetKey = $env:nugetKey
if(![string]::IsNullOrWhiteSpace($nugetKey))
{
    Write-Host "Enter to push nuget"
    pause
    Write-Host "enter to confirm"
    pause
    $files = [System.IO.Directory]::GetFiles(".\TqkLibrary.WindowCapture\bin\Release\")
    iex "nuget push $($files[0]) -ApiKey $($nugetKey) -Source https://api.nuget.org/v3/index.json"
}