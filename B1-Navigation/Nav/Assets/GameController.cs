using UnityEngine;
using System.Collections;

public class GameController : MonoBehaviour {
    private GameObject selectObject;
	// Use this for initialization
	void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
        if (Input.GetMouseButtonDown(0))
        {
            //Debug.Log("Left Click");
            RaycastHit hit;
            if (Physics.Raycast(Camera.main.ScreenPointToRay(Input.mousePosition), out hit, 100))
            {
                if (hit.transform.tag == "Player")
                {
                    if (selectObject != null)
                    {
                        Debug.Log("Player Deselected");
                        selectObject.SendMessage("Deselect", 1);
                    }
                    Debug.Log("Player Selected");
                    selectObject = hit.transform.gameObject;
                    selectObject.SendMessage("Select", 1);

                }
                if (hit.transform.tag == "Ground")
                {
                    Debug.Log("Player Deselected");
                    selectObject.SendMessage("Deselect", 1);
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
                if (hit.transform.tag != null && selectObject != null)
                {
                    Debug.Log("Destination Selected");
                    selectObject.SendMessage("Destination", hit.point);
                }
            }
        }
	}
}
