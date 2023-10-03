#pragma once
#include "pch.h"

/*
* Extended Object Data class
* Contains MapEditor specific object data
*
* FIXME: Every object should have this
* Class for storing object related data
* Contains eular & quaternion rotations
*/
class ExData {
  private:
    // Rotations are a mess
    CVector m_vecRot {0, 0, 0};
    struct QUAT {
        float x, y, z, w;
    } m_fQuat;

  public:
    int handle; // obj handle
    CObject *pObj; // pointer to obj
    std::string m_modelName; // dffName ; FIXME: store modloader ones

    // ---------------------------------------------------
    /*
    *   Game crashes to get or set quats of far away objects
    *   So we're basically storing them here with custom functions
    */
    void SetRotation(CVector rot, bool quat = true);
    CVector GetRotation();
    QUAT GetQuat();
    void SetQuat(QUAT quat);
    // ---------------------------------------------------

    ExData(CObject *obj) {
        pObj = obj;
        handle = CPools::GetObjectRef(obj);
    }
};

/*
* Object Manager Class
* Handles Opcodes Information
*/
class ObjectMgr {
  private:
    void HighlightObject(CEntity *pEntity);

  public:
    /*
    * Vector for stories entity model names
    * Format: model = dffName
    */
    std::vector<std::pair<std::string, std::vector<std::pair<int, std::string>>>> m_vecModelNames;
    std::vector<CObject*> m_pPlacedObjs; // Vector of currently placed entities
    ObjectExtendedData<ExData> m_objData; // Used to store extended data for each entity
    CObject* m_pSelected; // Currently selected entity, Points to nullptr if none is selected
    int m_nTotalIDELine; // Total number of ide lines

    struct{
        int m_nModel;
        CVector m_vecRot;
    } ClipBoard;

    ObjectMgr();
    ObjectMgr(ObjectMgr&);

    // Returns object name from it's model
    std::string FindNameFromModel(int model);

    // Returns the bottom Z bounding box coordinates
    float GetBoundingBoxGroundZ(CObject *pObj);
};

extern ObjectMgr ObjMgr;