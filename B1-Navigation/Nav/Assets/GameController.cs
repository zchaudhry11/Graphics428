using UnityEngine;
using System.Collections;

public class GameController : MonoBehaviour {
    private GameObject selectObject;
	// Use this for initialization
	void Start () {
        
	}
	
	// Update is called once per frame
	void Update () {
        //System.Collections.Generic.List<GameObject> players = new System.Collections.Generic.List<GameObject>();
        if (Input.GetMouseButtonDown(0))
        {
            //Debug.Log("Left Click");
            RaycastHit hit;
            if (Physics.Raycast(Camera.main.ScreenPointToRay(Input.mousePosition), out hit, 100))
            {
                if (hit.transform.tag == "Player")
                {
                    
                    Debug.Log("Player Selected");
                    selectObject = hit.transform.gameObject;
                    if (selectObject != null)
                    {
                        selectObject.tag = "Selected";
                        selectObject.SendMessage("Select", 1);
                    }

                }
				if(hit.transform.tag == "Cube")
				{
                    Debug.Log("Cube Selected");
					selectObject = hit.transform.gameObject;
					if(selectObject != null)
					{
						selectObject.SendMessage("Select");
					}
				}
				if (hit.transform.tag == "Ground" && selectObject != null)
                {
                    Debug.Log("Player Deselected");
                    GameObject[] deselect = GameObject.FindGameObjectsWithTag("Selected");
                    for (int i = 0; i < deselect.Length; i++)
                    {
                        deselect[i].SendMessage("Deselect", 1);
                        deselect[i].tag = "Player";
                    }
                }
                if (hit.transform.tag == null)
                {
                    Debug.Log("no tag found");
                }

            }
        }
        if (Input.GetMouseButtonDown(1))
        {
            //Debug.Log("Right Click");
            RaycastHit hit;
            if (Physics.Raycast(Camera.main.ScreenPointToRay(Input.mousePosition), out hit, 100))
            {
                if (hit.transform.tag != null && selectObject != null && GameObject.FindGameObjectsWithTag("Selected").Length != 0)
                {
                    Debug.Log("Destination Selected");
                    GameObject[] setDest = GameObject.FindGameObjectsWithTag("Selected");
                    for (int i = 0; i < setDest.Length; i++)
                    {
                        setDest[i].SendMessage("Destination", hit.point);
                    }
                    //selectObject.SendMessage("Destination", hit.point);
                }
            }
        }
	}
}
