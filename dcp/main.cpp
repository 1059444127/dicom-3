// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "precomp.h"

using namespace Application::Infrastructure;
using namespace DCM::Operations;

static const struct
{
    const wchar_t* Name;
    unsigned NArgs;
    bool IsRequired;
} DCPArguments[] =
{

{ L"--image-average",          0, false },
{ L"--image-concat",           0, false },
{ L"--image-convert-to-csv",   0, false },
{ L"--input-folder",           1, false },
{ L"--input-file",             1, false },
{ L"--image-gfactor",          3, false },
{ L"--image-snr",              2, false },
{ L"--image-ssim",             0, false },
{ L"--output-folder",          1, false },
{ L"--output-file",            1, false },
{ L"--normalize-image",        0, false },
{ L"--voxelize-mean",          3, false },
{ L"--voxelize-stddev",        3, false }

};

bool DirectoryExists(const std::wstring& path)
{
    DWORD dwAttrib = GetFileAttributes(path.c_str());

    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
        (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool FileExists(const std::wstring& path)
{
    DWORD dwAttrib = GetFileAttributes(path.c_str());

    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
        (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) == 0);
}

class DCPApplication : public ApplicationBase<DCPApplication>
{
    HRESULT EnsureOption(const wchar_t* pOptionName)
    {
        bool isSet;
        RETURN_IF_FAILED(IsOptionSet(pOptionName, &isSet));
        if (!isSet)
        {
            RETURN_IF_FAILED(PrintInvalidArgument(true, pOptionName, L"Option not set."));
            return E_FAIL;
        }

        return S_OK;
    }
public:

    template <OperationType TType, typename... TArgs>
    HRESULT RunOperation(TArgs&&... args)
    {
        CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        {
            Application::Infrastructure::DeviceResources resources;
            auto spOperation = MakeOperation<TType>(std::forward<TArgs>(args)...);

            LogOperation<TType>();

            spOperation->Run(resources);
        }
        CoUninitialize();
        return S_OK;
    }

    template <OperationType TType>
    HRESULT TryRunVoxelize()
    {
        static_assert(TType == OperationType::VoxelizeMeans ||
                      TType == OperationType::VoxelizeStdDev,
                      "TryRunVoxelize only accepts VoxelizeMeans or VoxelizeStdDev as its parameter.");

        const wchar_t* pVoxelizeParameter;
        if constexpr (TType == OperationType::VoxelizeMeans)
        {
            pVoxelizeParameter = L"--voxelize-mean";
        }
        else if constexpr(TType == OperationType::VoxelizeStdDev)
        {
            pVoxelizeParameter = L"--voxelize-stddev";
        }
        else
        {
            return E_FAIL;
        }

        bool isSet;
        unsigned x, y, z;
        if (SUCCEEDED(IsOptionSet(pVoxelizeParameter, &isSet)) && isSet &&
            SUCCEEDED(GetOptionParameterAt<0>(pVoxelizeParameter, &x)) &&
            SUCCEEDED(GetOptionParameterAt<1>(pVoxelizeParameter, &y)) &&
            SUCCEEDED(GetOptionParameterAt<2>(pVoxelizeParameter, &z)))
        {
            RETURN_IF_FAILED(EnsureOption(L"--input-folder"));
            std::wstring inputFolder;
            RETURN_IF_FAILED(GetOptionParameterAt<0>(L"--input-folder", &inputFolder));

            RETURN_IF_FAILED(EnsureOption(L"--output-file"));
            std::wstring outputFile;
            RETURN_IF_FAILED(GetOptionParameterAt<0>(L"--output-file", &outputFile));

            RETURN_IF_FAILED(RunOperation<TType>(inputFolder, outputFile, x, y, z));
            return S_OK;
        }
        return S_FALSE;
    }

    HRESULT Run()
    {
        {
            auto hr = TryRunVoxelize<OperationType::VoxelizeMeans>();
            RETURN_HR_IF(hr, hr == S_OK || FAILED(hr));
        }

        {
            auto hr = TryRunVoxelize<OperationType::VoxelizeStdDev>();
            RETURN_HR_IF(hr, hr == S_OK || FAILED(hr));
        }

        bool isSet;
        if (SUCCEEDED(IsOptionSet(L"--normalize-image", &isSet)) && isSet)
        {
            std::wstring inputFile;
            RETURN_IF_FAILED(GetOptionParameterAt<0>(L"--input-file", &inputFile));
            RETURN_IF_FAILED(RunOperation<OperationType::Normalize>());
            return S_OK;
        }

        if (SUCCEEDED(IsOptionSet(L"--image-average", &isSet)) && isSet)
        {
            RETURN_IF_FAILED(EnsureOption(L"--input-folder"));
            std::wstring inputFolder;
            RETURN_IF_FAILED(GetOptionParameterAt<0>(L"--input-folder", &inputFolder));
            RETURN_IF_FAILED(RunOperation<OperationType::AverageImages>());
            return S_OK;
        }

        if (SUCCEEDED(IsOptionSet(L"--image-concat", &isSet)) && isSet)
        {
            RETURN_IF_FAILED(EnsureOption(L"--input-folder"));
            std::wstring inputFolder;
            RETURN_IF_FAILED(GetOptionParameterAt<0>(L"--input-folder", &inputFolder));
            RETURN_IF_FAILED(RunOperation<OperationType::ConcatenateImages>());
            return S_OK;
        }

        if (SUCCEEDED(IsOptionSet(L"--image-convert-to-csv", &isSet)) && isSet)
        {
            RETURN_IF_FAILED(EnsureOption(L"--input-file"));
            std::wstring inputFile;
            RETURN_IF_FAILED(GetOptionParameterAt<0>(L"--input-file", &inputFile));
            RETURN_IF_FAILED(RunOperation<OperationType::ImageToCsv>());
            return S_OK;
        }

        if (SUCCEEDED(IsOptionSet(L"--image-snr", &isSet)) && isSet)
        {
            return S_OK;
        }

        if (SUCCEEDED(IsOptionSet(L"--image-gfactor", &isSet)) && isSet)
        {
            return S_OK;
        }

        if (SUCCEEDED(IsOptionSet(L"--image-ssim", &isSet)) && isSet)
        {
        }

        return E_FAIL;
    }

    HRESULT GetLengthOfArgumentsToFollow(wchar_t* pwzArgName, unsigned* nArgsToFollow)
    {
        RETURN_HR_IF_NULL(E_POINTER, nArgsToFollow);
        RETURN_HR_IF_NULL(E_POINTER, pwzArgName);

        *nArgsToFollow = 0;

        auto foundIt = 
            std::find_if(
                std::begin(DCPArguments),
                std::end(DCPArguments),
                [pwzArgName](const auto& argument)
                {
                    return _wcsicmp(argument.Name, pwzArgName) == 0;
                }
            );

        RETURN_HR_IF(E_INVALIDARG, foundIt == std::end(DCPArguments));

        // Set the number of return args to follow
        *nArgsToFollow = foundIt->NArgs;

        return S_OK;
    };

    HRESULT ValidateArgument(const wchar_t* pArgumentName, bool* isValid)
    {
        RETURN_HR_IF_NULL(E_INVALIDARG, pArgumentName);
        RETURN_HR_IF_NULL(E_POINTER, isValid);
        *isValid = true;

        if (_wcsicmp(pArgumentName, L"--input-folder") == 0)
        {
            std::wstring inputFolder;
            *isValid = SUCCEEDED(GetOptionParameterAt<0>(L"--input-folder", &inputFolder));
            RETURN_HR_IF_FALSE(S_OK, *isValid);
            *isValid = DirectoryExists(inputFolder);
            RETURN_HR_IF_FALSE(S_OK, *isValid);
        }

        if (_wcsicmp(pArgumentName, L"--input-file") == 0)
        {
            std::wstring inputFile;
            *isValid = SUCCEEDED(GetOptionParameterAt<0>(L"--input-file", &inputFile));
            RETURN_HR_IF_FALSE(S_OK, *isValid);
            *isValid = FileExists(inputFile);
            RETURN_HR_IF_FALSE(S_OK, *isValid);
        }

        if (_wcsicmp(pArgumentName, L"--voxelize-mean") == 0)
        {
            unsigned tmp;
            *isValid = SUCCEEDED(GetOptionParameterAt<0>(L"--voxelize-mean", &tmp));
            RETURN_HR_IF_FALSE(S_OK, *isValid);
            *isValid = SUCCEEDED(GetOptionParameterAt<1>(L"--voxelize-mean", &tmp));
            RETURN_HR_IF_FALSE(S_OK, *isValid);
            *isValid = SUCCEEDED(GetOptionParameterAt<2>(L"--voxelize-mean", &tmp));
            RETURN_HR_IF_FALSE(S_OK, *isValid);
        }

        if (_wcsicmp(pArgumentName, L"--voxelize-stddev") == 0)
        {
            unsigned tmp;
            *isValid = SUCCEEDED(GetOptionParameterAt<0>(L"--voxelize-stddev", &tmp));
            RETURN_HR_IF_FALSE(S_OK, *isValid);
            *isValid = SUCCEEDED(GetOptionParameterAt<1>(L"--voxelize-stddev", &tmp));
            RETURN_HR_IF_FALSE(S_OK, *isValid);
            *isValid = SUCCEEDED(GetOptionParameterAt<2>(L"--voxelize-stddev", &tmp));
            RETURN_HR_IF_FALSE(S_OK, *isValid);
        }
        return S_OK;
    }

    HRESULT PrintHelp()
    {
        static const auto HELP = LR"(
DCPApplication Reference (dcaapplication.exe)

Parameter               Usage
--------------------------------------------------------------------------------------------------------
--input-folder          The path (absolute: c:\..., or relative) to a folder.

                        Examples:
                        dcp.exe --dataPath "C:\foldername" [[OTHER PARAMETERS]]
                        dcp.exe --dataPath "C:\foldername\subfolder\" [[OTHER PARAMETERS]]
                        dcp.exe --dataPath "folder" [[OTHER PARAMETERS]]
                        dcp.exe --dataPath "folder\subdolder\" [[OTHER PARAMETERS]]

--voxelize-stddev       This parameter causes the tool to perform voxelization by computing pixel standard
                        deviations for each voxel region.

                        Usage:
                        [--voxelize-stddev dim_one_size dim_two_size dim_three_size]
                        
                        dim_*_size should all be specified in millimeters.

                        Other Required Parameters:
                            --input-folder
                            --output-file (The output file will use the PNG container format)

                        Example: Setting 2cm cubed voxel size
                        dcp.exe --voxelize-stddev 20 20 20 --input-folder images/folder1 --output-file folder1.stddev.png

--voxelize-mean         This parameter causes the tool to perform voxelization by computing pixel means
                        for each voxel region.

                        Usage:
                        [--voxelize-mean dim_one_size dim_two_size dim_three_size]
                        
                        dim_*_size should all be specified in millimeters.

                        Other Required Parameters:
                            --input-folder
                            --output-file (The output file will use the PNG container format)

                        Example: Setting 2cm cubed voxel size
                        dcp.exe --voxelize-mean 20 20 20 --input-folder images/folder1 --output-file folder1.means.png
)";
        wprintf(HELP);
        return S_OK;
    }
};


int main()
{
    return DCPApplication::Execute();
}

