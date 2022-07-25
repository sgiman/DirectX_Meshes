//-----------------------------------------------------------------------------
// File: Meshes.cpp (x86/x64) - base sample for mesh
//
// Описание:	Для расширенной геометрии большинство приложений 
//		предпочитают загружать предварительно созданные Меши из файла. 
//		К счастью, при использовании мешей D3DX делает большую часть работы.
//		Работа для этого, разбор файла геометрии и создание буферов вершин
//		(и индексные буферы) для нас. В этом руководстве показано, 
//		как использовать D3DXMES объект, включая его загрузку из файла и отрисовку. 
//		Только один D3DX не обрабатывает для нас материалы и текстуры для сетки,
//		поэтому обратите внимание, что мы должны обрабатывать их вручную.
//
//		Примечание: одна расширенная (но приятная) функция, которую мы здесь не показываем, 
//		заключается в том, что при клонировании меша мы можем указать FVF. Таким образом, 
//		независимо от того, как сетка создана, мы можем добавить/удалить нормали, 
//		добавить больше текстуры наборы координат (для мультитекстурирования) и т.д.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
// Modified by sgiman @ 2022, jul
// Visual Studio 2010 (2008)
// Microsoft DirectX SDK (June 2010)
//-----------------------------------------------------------------------------
#include <Windows.h>
#include <mmsystem.h>
#include <d3dx9.h>
#pragma warning( disable : 4996 ) // отключить устаревшее предупреждение
#include <strsafe.h>
#pragma warning( default : 4996 )



//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
LPDIRECT3D9         g_pD3D = NULL;				// Используется для создания D3DDevice
LPDIRECT3DDEVICE9   g_pd3dDevice = NULL;		// Наше устройство рендеринга

LPD3DXMESH          g_pMesh = NULL;				// Наш меш-объект в системной памяти (sysmem)
D3DMATERIAL9*       g_pMeshMaterials = NULL;	// Материалы для нашей сетки
LPDIRECT3DTEXTURE9* g_pMeshTextures = NULL;		// Текстуры для нашего меша
DWORD               g_dwNumMaterials = 0L;		// Количество материалов сетки



//-----------------------------------------------------------------------------
// Name: InitD3D()
// Desc: Инициализирует Direct3D
//-----------------------------------------------------------------------------
HRESULT InitD3D( HWND hWnd )
{
    // Создать объект D3D.
    if( NULL == ( g_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )	// Проверить устройство Direct3D   
        return E_FAIL;												// Error!						

    // Настройте структуру, используемую для создания D3DDevice. 
	// Так как мы сейчас используя более сложную геометрию, создадим устройство с zbuffer.
    D3DPRESENT_PARAMETERS d3dpp;				// Параметры
    ZeroMemory( &d3dpp, sizeof( d3dpp ) );		// Очистить графический буфер 	
    d3dpp.Windowed = TRUE;						// Окно
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;	// Эффект замены (SwapEffect) запретить
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;	// Формат заднего буфера (BackBufferFormat/фон) - НЕИЗВЕСТНЫЙ 
    d3dpp.EnableAutoDepthStencil = TRUE;		// Включить автоматический трафарет глубины (EnableAutoDepthStencil) - Включён
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;	// Автоматический формат трафарета глубины (AutoDepthStencilFormat) - FMT_D16

    // Создать D3D-устройство
    if( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
                                      D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                      &d3dpp, &g_pd3dDevice ) ) )
    {
        return E_FAIL;
    }

    // Включить zbuffer
    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );

    // Включить окружающее освещение 
    g_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0xffffffff );

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: InitGeometry()
// Desc: Загрузить сетку и создать массивы материалов и текстур.
//-----------------------------------------------------------------------------
HRESULT InitGeometry()
{
    LPD3DXBUFFER pD3DXMtrlBuffer;	// 3DХ буффер

    //////////////////////////////////////// 
	// Загрузить сетку из указанного файла
    if( FAILED( D3DXLoadMeshFromX( L"Tiger.x", D3DXMESH_SYSTEMMEM,
                                   g_pd3dDevice, NULL,
                                   &pD3DXMtrlBuffer, NULL, &g_dwNumMaterials,
                                   &g_pMesh ) ) )
    {
        ///////////////////////////////////////////////////////////////////////////
		// Если модель не находится в текущей папке, попробуйте родительскую папку
        if( FAILED( D3DXLoadMeshFromX( L"..\\Tiger.x", D3DXMESH_SYSTEMMEM,
                                       g_pd3dDevice, NULL,
                                       &pD3DXMtrlBuffer, NULL, &g_dwNumMaterials,
                                       &g_pMesh ) ) )
        {
            MessageBox( NULL, L"Could not find tiger.x", L"Meshes.exe", MB_OK );
            return E_FAIL;
        }
    }

    // Нам нужно извлечь свойства материала и имена текстур из
    // pD3DXMtrlBuffer
    D3DXMATERIAL* d3dxMaterials = ( D3DXMATERIAL* )pD3DXMtrlBuffer->GetBufferPointer();
    g_pMeshMaterials = new D3DMATERIAL9[g_dwNumMaterials];
    if( g_pMeshMaterials == NULL )
        return E_OUTOFMEMORY;
    g_pMeshTextures = new LPDIRECT3DTEXTURE9[g_dwNumMaterials];
    if( g_pMeshTextures == NULL )
        return E_OUTOFMEMORY;

    for( DWORD i = 0; i < g_dwNumMaterials; i++ )
    {
        // Скопировать материал
        g_pMeshMaterials[i] = d3dxMaterials[i].MatD3D;

        // Установить окружающий цвет для материала (D3DX этого не делает)
        g_pMeshMaterials[i].Ambient = g_pMeshMaterials[i].Diffuse;

        g_pMeshTextures[i] = NULL;
        if( d3dxMaterials[i].pTextureFilename != NULL &&
            lstrlenA( d3dxMaterials[i].pTextureFilename ) > 0 )
        {
            // Создать текстуру
            if( FAILED( D3DXCreateTextureFromFileA( g_pd3dDevice,
                                                    d3dxMaterials[i].pTextureFilename,
                                                    &g_pMeshTextures[i] ) ) )
            {
                //////////////////////////////////////////////////////////////////// 
				//Если текстуры нет в текущей папке, попробуйте родительскую папку
                const CHAR* strPrefix = "..\\";
                CHAR strTexture[MAX_PATH];
                strcpy_s( strTexture, MAX_PATH, strPrefix );
                strcat_s( strTexture, MAX_PATH, d3dxMaterials[i].pTextureFilename );
                /////////////////////////////////////////////////////////////////////
				// Если текстуры нет в текущей папке, попробуйте родительскую папку
                if( FAILED( D3DXCreateTextureFromFileA( g_pd3dDevice,
                                                        strTexture,
                                                        &g_pMeshTextures[i] ) ) )
                {
                    MessageBox( NULL, L"Could not find texture map", L"Meshes.exe", MB_OK );
                }
            }
        }
    }

    // Готово с буфером материала
    pD3DXMtrlBuffer->Release();

    return S_OK;
}



//-----------------------------------------------------------------------------
// Name: Cleanup()
// Desc: Освобождает все ранее инициализированные объекты
//-----------------------------------------------------------------------------
VOID Cleanup()
{
    if( g_pMeshMaterials != NULL )
        delete[] g_pMeshMaterials;

    if( g_pMeshTextures )
    {
        for( DWORD i = 0; i < g_dwNumMaterials; i++ )
        {
            if( g_pMeshTextures[i] )
                g_pMeshTextures[i]->Release();
        }
        delete[] g_pMeshTextures;
    }
    if( g_pMesh != NULL )
        g_pMesh->Release();

    if( g_pd3dDevice != NULL )
        g_pd3dDevice->Release();

    if( g_pD3D != NULL )
        g_pD3D->Release();
}



//-----------------------------------------------------------------------------
// Name: SetupMatrices()
// Desc: Настраивает матрицы преобразования мира, вида и проекции.
//-----------------------------------------------------------------------------
VOID SetupMatrices()
{
    // Настроить мировую матрицу
    D3DXMATRIXA16 matWorld;
    D3DXMatrixRotationY( &matWorld, timeGetTime() / 1000.0f );
    g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

    // Настраиваем нашу матрицу представления. Матрица вида может быть определена с учетом точки обзора,
    // точка, на которую нужно смотреть, и направление, по которому путь вверх. 
    // Здесь мы устанавливаем смотрим на пять единиц назад по оси Z и на три единицы вверх, 
    // смотрим на начало координат и определить «вверх» в направлении оси y.    
	D3DXVECTOR3 vEyePt( 0.0f, 3.0f,-5.0f );
    D3DXVECTOR3 vLookatPt( 0.0f, 0.0f, 0.0f );
    D3DXVECTOR3 vUpVec( 0.0f, 1.0f, 0.0f );
    D3DXMATRIXA16 matView;
    D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );
    g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

    // Для матрицы проекции мы устанавливаем перспективное преобразование (которое
    // преобразует геометрию из 3D-пространства просмотра в 2D-пространство окна просмотра с
    // перспективное деление, делающее объекты на расстоянии меньше). 
    // Строитьпреобразование перспективы, нам нужно поле зрения (обычно 1/4 pi),
    // соотношение сторон и ближняя и дальняя плоскости отсечения 
    // (определяемые в геометрия каких расстояний больше не должна отображаться).
    D3DXMATRIXA16 matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );
}



//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Рисует сцену
//-----------------------------------------------------------------------------
VOID Render()
{
    // Очистить задний буфер и zbuffer
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                         D3DCOLOR_XRGB( 0, 0, 255 ), 1.0f, 0 );

    // Начать сцену
    if( SUCCEEDED( g_pd3dDevice->BeginScene() ) )
    {
        // Настроить мир, представление и проекционные матрицы
        SetupMatrices();

        // Meshes are divided into subsets, one for each material. Render them in
        // a loop

		// Меши разделены на подмножества, по одному для каждого материала. 
        // Визуализировать их в цикле
        for( DWORD i = 0; i < g_dwNumMaterials; i++ )
        {
            // Установить материал и текстуру для этого подмножества
            g_pd3dDevice->SetMaterial( &g_pMeshMaterials[i] );
            g_pd3dDevice->SetTexture( 0, g_pMeshTextures[i] );

            // Нарисуйте подмножество сетки
            g_pMesh->DrawSubset( i );
        }

        // Завершить сцену
        g_pd3dDevice->EndScene();
    }

    // Представить содержимое обратного буфера на дисплей
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}



//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: Обработчик сообщений окна
//-----------------------------------------------------------------------------
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
        case WM_DESTROY:
            Cleanup();
            PostQuitMessage( 0 );
            return 0;
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}



//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Точка входа приложения
//-----------------------------------------------------------------------------
INT WINAPI wWinMain( HINSTANCE hInst, HINSTANCE, LPWSTR, INT )
{
    UNREFERENCED_PARAMETER( hInst );

    // Зарегистрировать класс окна
    WNDCLASSEX wc =
    {
        sizeof( WNDCLASSEX ), CS_CLASSDC, MsgProc, 0L, 0L,
        GetModuleHandle( NULL ), NULL, NULL, NULL, NULL,
        L"D3D Tutorial", NULL
    };
    RegisterClassEx( &wc );

    // Создать окно приложения
    HWND hWnd = CreateWindow( L"D3D Tutorial", L"D3D Tutorial 06: Meshes",
                              WS_OVERLAPPEDWINDOW, 100, 100, 300, 300,
                              NULL, NULL, wc.hInstance, NULL );

    // Инициализировать Direct3D
    if( SUCCEEDED( InitD3D( hWnd ) ) )
    {
        // Создайть геометрию сцены
        if( SUCCEEDED( InitGeometry() ) )
        {
            // Показать окно
            ShowWindow( hWnd, SW_SHOWDEFAULT );
            UpdateWindow( hWnd );

            // Войти в цикл сообщений
            MSG msg;
            ZeroMemory( &msg, sizeof( msg ) );
            while( msg.message != WM_QUIT )
            {
                if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
                {
                    TranslateMessage( &msg );
                    DispatchMessage( &msg );
                }
                else
                    Render();
            }
        }
    }

    UnregisterClass( L"D3D Tutorial", wc.hInstance );
    return 0;
}
