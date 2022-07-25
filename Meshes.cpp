//-----------------------------------------------------------------------------
// File: Meshes.cpp (x86/x64) - base sample for mesh
//
// ��������:	��� ����������� ��������� ����������� ���������� 
//				������������ ��������� �������������� ��������� ���� �� �����. 
//				� �������, ��� ������������� ����� D3DX ������ ������� ����� ������.
//				������ ��� �����, ������ ����� ��������� � �������� ������� ������
//				(� ��������� ������) ��� ���. � ���� ����������� ��������, 
//				��� ������������ D3DXMES ������, ������� ��� �������� �� ����� � ���������. 
//				������ ���� D3DX �� ������������ ��� ��� ��������� � �������� ��� �����,
//				������� �������� ��������, ��� �� ������ ������������ �� �������.
//
//				����������: ���� ����������� (�� ��������) �������, ������� �� ����� �� ����������, 
//				����������� � ���, ��� ��� ������������ ���� �� ����� ������� FVF. ����� �������, 
//				���������� �� ����, ��� ����� �������, �� ����� ��������/������� �������, 
//				�������� ������ �������� ������ ��������� (��� ���������������������) � �.�.
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
#pragma warning( disable : 4996 ) // ��������� ���������� ��������������
#include <strsafe.h>
#pragma warning( default : 4996 )




//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
LPDIRECT3D9         g_pD3D = NULL;				// ������������ ��� �������� D3DDevice
LPDIRECT3DDEVICE9   g_pd3dDevice = NULL;		// ���� ���������� ����������

LPD3DXMESH          g_pMesh = NULL;				// ��� ���-������ � ��������� ������ (sysmem)
D3DMATERIAL9*       g_pMeshMaterials = NULL;	// ��������� ��� ����� �����
LPDIRECT3DTEXTURE9* g_pMeshTextures = NULL;		// �������� ��� ������ ����
DWORD               g_dwNumMaterials = 0L;		// ���������� ���������� �����




//-----------------------------------------------------------------------------
// Name: InitD3D()
// Desc: �������������� Direct3D
//-----------------------------------------------------------------------------
HRESULT InitD3D( HWND hWnd )
{
    // ������� ������ D3D.
    if( NULL == ( g_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )	// ��������� ���������� Direct3D   
        return E_FAIL;												// Error!						

    // ��������� ���������, ������������ ��� �������� D3DDevice. 
	// ��� ��� �� ������ ��������� ����� ������� ���������, �������� ���������� � zbuffer.
    D3DPRESENT_PARAMETERS d3dpp;				// ���������
    ZeroMemory( &d3dpp, sizeof( d3dpp ) );		// �������� ����������� ����� 	
    d3dpp.Windowed = TRUE;						// ����
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;	// ������ ������ (SwapEffect) ���������
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;	// ������ ������� ������ (BackBufferFormat/���) - ����������� 
    d3dpp.EnableAutoDepthStencil = TRUE;		// �������� �������������� �������� ������� (EnableAutoDepthStencil) - �������
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;	// �������������� ������ ��������� ������� (AutoDepthStencilFormat) - FMT_D16

    // ������� D3D-����������
    if( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
                                      D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                      &d3dpp, &g_pd3dDevice ) ) )
    {
        return E_FAIL;
    }

    // �������� zbuffer
    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );

    // �������� ���������� ��������� 
    g_pd3dDevice->SetRenderState( D3DRS_AMBIENT, 0xffffffff );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InitGeometry()
// Desc: ��������� ����� � ������� ������� ���������� � �������.
//-----------------------------------------------------------------------------
HRESULT InitGeometry()
{
    LPD3DXBUFFER pD3DXMtrlBuffer;	// 3D� ������

    //////////////////////////////////////// 
	// ��������� ����� �� ���������� �����
    if( FAILED( D3DXLoadMeshFromX( L"Tiger.x", D3DXMESH_SYSTEMMEM,
                                   g_pd3dDevice, NULL,
                                   &pD3DXMtrlBuffer, NULL, &g_dwNumMaterials,
                                   &g_pMesh ) ) )
    {
        ///////////////////////////////////////////////////////////////////////////
		// ���� ������ �� ��������� � ������� �����, ���������� ������������ �����
        if( FAILED( D3DXLoadMeshFromX( L"..\\Tiger.x", D3DXMESH_SYSTEMMEM,
                                       g_pd3dDevice, NULL,
                                       &pD3DXMtrlBuffer, NULL, &g_dwNumMaterials,
                                       &g_pMesh ) ) )
        {
            MessageBox( NULL, L"Could not find tiger.x", L"Meshes.exe", MB_OK );
            return E_FAIL;
        }
    }

    // ��� ����� ������� �������� ��������� � ����� ������� ��
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
        // ����������� ��������
        g_pMeshMaterials[i] = d3dxMaterials[i].MatD3D;

        // ���������� ���������� ���� ��� ��������� (D3DX ����� �� ������)
        g_pMeshMaterials[i].Ambient = g_pMeshMaterials[i].Diffuse;

        g_pMeshTextures[i] = NULL;
        if( d3dxMaterials[i].pTextureFilename != NULL &&
            lstrlenA( d3dxMaterials[i].pTextureFilename ) > 0 )
        {
            // ������� ��������
            if( FAILED( D3DXCreateTextureFromFileA( g_pd3dDevice,
                                                    d3dxMaterials[i].pTextureFilename,
                                                    &g_pMeshTextures[i] ) ) )
            {
                //////////////////////////////////////////////////////////////////// 
				//���� �������� ��� � ������� �����, ���������� ������������ �����
                const CHAR* strPrefix = "..\\";
                CHAR strTexture[MAX_PATH];
                strcpy_s( strTexture, MAX_PATH, strPrefix );
                strcat_s( strTexture, MAX_PATH, d3dxMaterials[i].pTextureFilename );
                /////////////////////////////////////////////////////////////////////
				// ���� �������� ��� � ������� �����, ���������� ������������ �����
                if( FAILED( D3DXCreateTextureFromFileA( g_pd3dDevice,
                                                        strTexture,
                                                        &g_pMeshTextures[i] ) ) )
                {
                    MessageBox( NULL, L"Could not find texture map", L"Meshes.exe", MB_OK );
                }
            }
        }
    }

    // ������ � ������� ���������
    pD3DXMtrlBuffer->Release();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: Cleanup()
// Desc: ����������� ��� ����� ������������������ �������
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
// Desc: ����������� ������� �������������� ����, ���� � ��������.
//-----------------------------------------------------------------------------
VOID SetupMatrices()
{
    // ��������� ������� �������
    D3DXMATRIXA16 matWorld;
    D3DXMatrixRotationY( &matWorld, timeGetTime() / 1000.0f );
    g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

	// ����������� ���� ������� �������������. ������� ���� ����� ���� ���������� � ������ ����� ������,
    // �����, �� ������� ����� ��������, � �����������, �� �������� ���� �����. 
    // ����� �� ������������� ������� �� ���� ������ ����� �� ��� Z � �� ��� ������� �����, 
    // ������� �� ������ ��������� � ���������� ������� � ����������� ��� y.    
	D3DXVECTOR3 vEyePt( 0.0f, 3.0f,-5.0f );
    D3DXVECTOR3 vLookatPt( 0.0f, 0.0f, 0.0f );
    D3DXVECTOR3 vUpVec( 0.0f, 1.0f, 0.0f );
    D3DXMATRIXA16 matView;
    D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );
    g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

	// ��� ������� �������� �� ������������� ������������� �������������� (�������
    // ����������� ��������� �� 3D-������������ ��������� � 2D-������������ ���� ��������� �
    // ������������� �������, �������� ������� �� ���������� ������). 
    // ��������������������� �����������, ��� ����� ���� ������ (������ 1/4 pi),
    // ����������� ������ � ������� � ������� ��������� ��������� 
    // (������������ � ��������� ����� ���������� ������ �� ������ ������������).
    D3DXMATRIXA16 matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );
}




//-----------------------------------------------------------------------------
// Name: Render()
// Desc: ������ �����
//-----------------------------------------------------------------------------
VOID Render()
{
    // �������� ������ ����� � zbuffer
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                         D3DCOLOR_XRGB( 0, 0, 255 ), 1.0f, 0 );

    // ������ �����
    if( SUCCEEDED( g_pd3dDevice->BeginScene() ) )
    {
        // ��������� ���, ������������� � ������������ �������
        SetupMatrices();

        // Meshes are divided into subsets, one for each material. Render them in
        // a loop

		// ���� ��������� �� ������������, �� ������ ��� ������� ���������. 
        // ��������������� �� � �����
        for( DWORD i = 0; i < g_dwNumMaterials; i++ )
        {
            // ���������� �������� � �������� ��� ����� ������������
            g_pd3dDevice->SetMaterial( &g_pMeshMaterials[i] );
            g_pd3dDevice->SetTexture( 0, g_pMeshTextures[i] );

            // ��������� ������������ �����
            g_pMesh->DrawSubset( i );
        }

        // ��������� �����
        g_pd3dDevice->EndScene();
    }

    // ����������� ���������� ��������� ������ �� �������
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}




//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: ���������� ��������� ����
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
// Desc: ����� ����� ����������
//-----------------------------------------------------------------------------
INT WINAPI wWinMain( HINSTANCE hInst, HINSTANCE, LPWSTR, INT )
{
    UNREFERENCED_PARAMETER( hInst );

    // ���������������� ����� ����
    WNDCLASSEX wc =
    {
        sizeof( WNDCLASSEX ), CS_CLASSDC, MsgProc, 0L, 0L,
        GetModuleHandle( NULL ), NULL, NULL, NULL, NULL,
        L"D3D Tutorial", NULL
    };
    RegisterClassEx( &wc );

    // ������� ���� ����������
    HWND hWnd = CreateWindow( L"D3D Tutorial", L"D3D Tutorial 06: Meshes",
                              WS_OVERLAPPEDWINDOW, 100, 100, 300, 300,
                              NULL, NULL, wc.hInstance, NULL );

    // ���������������� Direct3D
    if( SUCCEEDED( InitD3D( hWnd ) ) )
    {
        // �������� ��������� �����
        if( SUCCEEDED( InitGeometry() ) )
        {
            // �������� ����
            ShowWindow( hWnd, SW_SHOWDEFAULT );
            UpdateWindow( hWnd );

            // ����� � ���� ���������
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