#include "FileDialog.hpp"
#include <stdexcept>
#include "../string/StringUtil.hpp"

namespace nfd {
	FileDialog::FileDialog()
	{
		HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to initialize COM");
		}
	}
	FileDialog::~FileDialog()
	{
		CoUninitialize();
	}

	/*
	 // Prepare file type filters
	std::vector<std::pair<std::wstring, std::wstring>> fileTypes = {
		{ L"Text Files (*.txt)", L"*.txt" },
		{ L"Image Files (*.png;*.jpg;*.bmp)", L"*.png;*.jpg;*.bmp" },
		{ L"All Files (*.*)", L"*.*" }
	};

	try
	{
		std::string filePath = dialog.OpenFileDialog(fileTypes);
		MessageBoxA(NULL, filePath.c_str(), "Selected File", MB_OK);
	}
	catch (const std::exception& e)
	{
		MessageBoxA(NULL, e.what(), "Error", MB_OK | MB_ICONERROR);
	}

	*/

	std::string FileDialog::openFileDialog(const std::vector<std::pair<std::wstring, std::wstring>>& fileTypes) const
	{
		IFileOpenDialog* pFileOpen = nullptr;

		// Create the FileOpenDialog object
		HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to create File Open Dialog");
		}

		// Prepare COMDLG_FILTERSPEC array from the input fileTypes
		std::vector<COMDLG_FILTERSPEC> filterSpec(fileTypes.size());
		for (size_t i = 0; i < fileTypes.size(); ++i)
		{
			filterSpec[i].pszName = fileTypes[i].first.c_str(); // Description (e.g., "Text Files (*.txt)")
			filterSpec[i].pszSpec = fileTypes[i].second.c_str(); // Filter pattern (e.g., "*.txt")
		}

		// Set file type filters
		hr = pFileOpen->SetFileTypes(static_cast<UINT>(filterSpec.size()), filterSpec.data());
		if (FAILED(hr))
		{
			pFileOpen->Release();
			throw std::runtime_error("Failed to set file filters");
		}

		// Set the default file type index (optional)
		hr = pFileOpen->SetFileTypeIndex(1);  // 1-based index, here it will default to the first filter
		if (FAILED(hr))
		{
			pFileOpen->Release();
			throw std::runtime_error("Failed to set default file type");
		}

		// Show the Open dialog box
		hr = pFileOpen->Show(NULL);
		if (FAILED(hr))
		{
			pFileOpen->Release();
			throw std::runtime_error("No file was selected or dialog failed");
		}

		// Get the file name from the dialog box
		IShellItem* pItem = nullptr;
		hr = pFileOpen->GetResult(&pItem);
		if (FAILED(hr))
		{
			pFileOpen->Release();
			throw std::runtime_error("Failed to retrieve file result");
		}

		// Extract the file path
		PWSTR pszFilePath = nullptr;
		hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
		if (FAILED(hr))
		{
			pItem->Release();
			pFileOpen->Release();
			throw std::runtime_error("Failed to get file path");
		}

		// Convert the wide string (WCHAR) to a standard string (char)
		std::string filePath = StringUtil::WideStringToString(pszFilePath);

		// Free memory and release resources
		CoTaskMemFree(pszFilePath);
		pItem->Release();
		pFileOpen->Release();

		return filePath;
	}
	std::vector<std::string> FileDialog::multiSelectFileDialog(const std::vector<std::pair<std::wstring, std::wstring>>& fileTypes) const
	{
		IFileOpenDialog* pFileOpen = nullptr;

		// Create the FileOpenDialog object
		HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to create File Open Dialog");
		}

		// Enable multi-selection
		DWORD dwFlags;
		hr = pFileOpen->GetOptions(&dwFlags);
		if (SUCCEEDED(hr))
		{
			hr = pFileOpen->SetOptions(dwFlags | FOS_ALLOWMULTISELECT);
		}
		if (FAILED(hr))
		{
			pFileOpen->Release();
			throw std::runtime_error("Failed to enable multi-select");
		}

		// Prepare COMDLG_FILTERSPEC array from the input fileTypes
		std::vector<COMDLG_FILTERSPEC> filterSpec(fileTypes.size());
		for (size_t i = 0; i < fileTypes.size(); ++i)
		{
			filterSpec[i].pszName = fileTypes[i].first.c_str(); // Description (e.g., "Text Files (*.txt)")
			filterSpec[i].pszSpec = fileTypes[i].second.c_str(); // Filter pattern (e.g., "*.txt")
		}

		// Set file type filters
		hr = pFileOpen->SetFileTypes(static_cast<UINT>(filterSpec.size()), filterSpec.data());
		if (FAILED(hr))
		{
			pFileOpen->Release();
			throw std::runtime_error("Failed to set file filters");
		}

		// Set the default file type index (optional)
		hr = pFileOpen->SetFileTypeIndex(1);  // 1-based index, here it will default to the first filter
		if (FAILED(hr))
		{
			pFileOpen->Release();
			throw std::runtime_error("Failed to set default file type");
		}

		// Show the Open dialog box
		hr = pFileOpen->Show(NULL);
		if (FAILED(hr))
		{
			pFileOpen->Release();
			throw std::runtime_error("No file was selected or dialog failed");
		}

		// Get the file names from the dialog box
		IShellItemArray* pItemArray = nullptr;
		hr = pFileOpen->GetResults(&pItemArray);
		if (FAILED(hr))
		{
			pFileOpen->Release();
			throw std::runtime_error("Failed to retrieve file results");
		}

		// Extract the file paths
		std::vector<std::string> filePaths;
		DWORD numItems = 0;
		hr = pItemArray->GetCount(&numItems);
		if (SUCCEEDED(hr))
		{
			for (DWORD i = 0; i < numItems; ++i)
			{
				IShellItem* pItem = nullptr;
				hr = pItemArray->GetItemAt(i, &pItem);
				if (SUCCEEDED(hr))
				{
					PWSTR pszFilePath = nullptr;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
					if (SUCCEEDED(hr))
					{
						// Convert the wide string (WCHAR) to a standard string (char)
						std::string filePath = StringUtil::WideStringToString(pszFilePath);
						filePaths.push_back(filePath);
						CoTaskMemFree(pszFilePath);
					}
					pItem->Release();
				}
			}
		}

		// Release resources
		pItemArray->Release();
		pFileOpen->Release();

		return filePaths;
	}
}
